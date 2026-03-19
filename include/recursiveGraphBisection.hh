#pragma once

#include <cmath>
#include <iterator>
#include <vector>

#include "tbb/enumerable_thread_specific.h"
#include "tbb/parallel_invoke.h"
#include "tbb/task_group.h"

#include "util/compilerAttribute.hh"
#include "util/forwardIndex.hh"
#include "util/log.hh"
#include "util/singleInitVector.hh"

namespace pisa {
const Log2<4096> log2;

namespace bp {

    using ThreadLocalGains = tbb::enumerable_thread_specific<singleInitVector<double>>;
    using ThreadLocalDegrees = tbb::enumerable_thread_specific<singleInitVector<size_t>>;

    struct ThreadLocal {
        ThreadLocalGains gains;
        ThreadLocalDegrees left_degrees;
        ThreadLocalDegrees right_degrees;
    };

    ALWAYSINLINE double expb(double logn1, double logn2, size_t deg1, size_t deg2) {
        return static_cast<double>(deg1) * logn1
             - static_cast<double>(deg1) * log2(static_cast<double>(deg1) + 1.0)
             + static_cast<double>(deg2) * logn2
             - static_cast<double>(deg2) * log2(static_cast<double>(deg2) + 1.0);
    };

    template <typename ThreadLocalContainer>
    [[nodiscard]] ALWAYSINLINE auto&
    clearOrInit(ThreadLocalContainer&& container, std::size_t size) {
        bool exists = false;
        auto& ref = container.local(exists);
        if (exists) {
            ref.clear();
        } else {
            ref.resize(size);
        }
        return ref;
    }

}  // namespace bp

template <class Iterator>
struct verticePartition;

template <class Iterator>
class verticeRange {
  public:
    using value_type = typename std::iterator_traits<Iterator>::value_type;

    verticeRange(
        Iterator first,
        Iterator last,
        std::reference_wrapper<const forwardIndex> fwdidx,
        std::reference_wrapper<std::vector<double>> gains
    )
        : m_first(first), m_last(last), m_fwdidx(fwdidx), m_gains(gains) {}

    Iterator begin() { return m_first; }
    Iterator end() { return m_last; }
    std::ptrdiff_t size() const { return std::distance(m_first, m_last); }

    ALWAYSINLINE verticePartition<Iterator> split() const {
        Iterator mid = std::next(m_first, size() / 2);
        return {
            verticeRange(m_first, mid, m_fwdidx, m_gains),
            verticeRange(mid, m_last, m_fwdidx, m_gains),
            term_count()
        };
    }

    ALWAYSINLINE verticeRange operator()(std::ptrdiff_t left, std::ptrdiff_t right) const {
        assert(left < right);
        assert(right <= size());
        return verticeRange(std::next(m_first, left), std::next(m_first, right), m_fwdidx, m_gains);
    }

    std::size_t term_count() const { return m_fwdidx.get().termCount(); }
    std::vector<uint32_t> terms(value_type vertice) const {
        return m_fwdidx.get().terms(vertice);
    }
    double gain(value_type vertice) const { return m_gains.get()[vertice]; }
    double& gain(value_type vertice) { return m_gains.get()[vertice]; }

    auto by_gain() {
        return [this](const value_type& lhs, const value_type& rhs) {
            return m_gains.get()[lhs] > m_gains.get()[rhs];
        };
    }

  private:
    Iterator m_first;
    Iterator m_last;
    std::reference_wrapper<const forwardIndex> m_fwdidx;
    std::reference_wrapper<std::vector<double>> m_gains;
};

template <class Iterator>
struct verticePartition {
    verticeRange<Iterator> left;
    verticeRange<Iterator> right;
    size_t term_count;

    std::ptrdiff_t size() const { return left.size() + right.size(); }
};

template <class Iterator>
void computeDegrees(verticeRange<Iterator>& range, singleInitVector<size_t>& deg_map) {
    for (const auto& vertice: range) {
        auto terms = range.terms(vertice);
        auto deg_map_inc = [&](const auto& t) { deg_map.set(t, deg_map[t] + 1); };
        std::for_each(terms.begin(), terms.end(), deg_map_inc);
    }
}

template <bool isLikelyCached = true, typename Iter>
void computeMoveGainsCaching(
    verticeRange<Iter>& range,
    const std::ptrdiff_t from_n,
    const std::ptrdiff_t to_n,
    const singleInitVector<size_t>& from_lex,
    const singleInitVector<size_t>& to_lex,
    bp::ThreadLocal& thread_local_data
) {
    const auto logn1 = log2(from_n);
    const auto logn2 = log2(to_n);

    auto& gain_cache = bp::clearOrInit(thread_local_data.gains, from_lex.size());
    auto computeVerticeGain = [&](auto& d) {
        double gain = 0.0;
        auto terms = range.terms(d);
        for (const auto& t: terms) {
            if constexpr (isLikelyCached) {  // NOLINT(readability-braces-around-statements)
                if (not gain_cache.has_value(t)) [[unlikely]] {
                    const auto& from_deg = from_lex[t];
                    const auto& to_deg = to_lex[t];
                    const auto term_gain = bp::expb(logn1, logn2, from_deg, to_deg)
                        - bp::expb(logn1, logn2, from_deg - 1, to_deg + 1);
                    gain_cache.set(t, term_gain);
                }
            } else {
                if (not gain_cache.has_value(t)) [[likely]] {
                    const auto& from_deg = from_lex[t];
                    const auto& to_deg = to_lex[t];
                    const auto term_gain = bp::expb(logn1, logn2, from_deg, to_deg)
                        - bp::expb(logn1, logn2, from_deg - 1, to_deg + 1);
                    gain_cache.set(t, term_gain);
                }
            }
            gain += gain_cache[t];
        }
        range.gain(d) = gain;
    };
    std::for_each(range.begin(), range.end(), computeVerticeGain);
}

template <class Iterator, class GainF>
void computeGains(
    verticePartition<Iterator>& partition,
    const degreeMapPair& degrees,
    GainF gainFunction,
    bp::ThreadLocal& thread_local_data
) {
    auto n1 = partition.left.size();
    auto n2 = partition.right.size();
    gainFunction(partition.left, n1, n2, degrees.left, degrees.right, thread_local_data);
    gainFunction(partition.right, n2, n1, degrees.right, degrees.left, thread_local_data);
}

template <class Iterator>
void swap(verticePartition<Iterator>& partition, degreeMapPair& degrees) {
    auto left = partition.left;
    auto right = partition.right;
    auto lit = left.begin();
    auto rit = right.begin();
    for (; lit != left.end() && rit != right.end(); ++lit, ++rit) {
        if (left.gain(*lit) + right.gain(*rit) <= 0) [[unlikely]] {
            break;
        }
        {
            auto terms = left.terms(*lit);
            for (auto& term: terms) {
                degrees.left.set(term, degrees.left[term] - 1);
                degrees.right.set(term, degrees.right[term] + 1);
            }
        }
        {
            auto terms = right.terms(*rit);
            for (auto& term: terms) {
                degrees.left.set(term, degrees.left[term] + 1);
                degrees.right.set(term, degrees.right[term] - 1);
            }
        }

        std::iter_swap(lit, rit);
    }
}

template <class Iterator, class GainF>
void processPartition(
    verticePartition<Iterator>& partition,
    GainF gainFunction,
    bp::ThreadLocal& thread_local_data,
    int iterations = 20
) {
    auto& left_degree =
        bp::clearOrInit(thread_local_data.left_degrees, partition.left.term_count());
    auto& right_degree =
        bp::clearOrInit(thread_local_data.right_degrees, partition.right.term_count());
    computeDegrees(partition.left, left_degree);
    computeDegrees(partition.right, right_degree);
    degreeMapPair degrees{left_degree, right_degree};

    for (int iteration = 0; iteration < iterations; ++iteration) {
        computeGains(partition, degrees, gainFunction, thread_local_data);
        tbb::parallel_invoke(
            [&] {
                std::sort(
                    partition.left.begin(),
                    partition.left.end(),
                    partition.left.by_gain()
                );
            },
            [&] {
                std::sort(
                    partition.right.begin(),
                    partition.right.end(),
                    partition.right.by_gain()
                );
            }
        );
        swap(partition, degrees);
    }
}

template <class Iterator>
void recursiveGraphBisection(
    verticeRange<Iterator> vertices,
    size_t depth,
    int iterations,
    size_t cache_depth,
    std::shared_ptr<bp::ThreadLocal> thread_local_data = nullptr
) {
    if (thread_local_data == nullptr) {
        thread_local_data = std::make_shared<bp::ThreadLocal>();
    }
    std::sort(vertices.begin(), vertices.end());
    auto partition = vertices.split();
    if (cache_depth >= 1) {
        processPartition(partition, computeMoveGainsCaching<true, Iterator>, *thread_local_data, iterations);
        --cache_depth;
    } else {
        processPartition(partition, computeMoveGainsCaching<false, Iterator>, *thread_local_data, iterations);
    }

    if (depth > 1 && vertices.size() > 2) {
        tbb::parallel_invoke(
            [&, thread_local_data] {
                recursiveGraphBisection(partition.left, depth - 1, iterations, cache_depth, thread_local_data);
            },
            [&, thread_local_data] {
                recursiveGraphBisection(partition.right, depth - 1, iterations, cache_depth, thread_local_data);
            }
        );
    } else {
        std::sort(partition.left.begin(), partition.left.end());
        std::sort(partition.right.begin(), partition.right.end());
    }
}

}  // namespace pisa