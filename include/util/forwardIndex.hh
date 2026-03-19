#pragma once

#include <cstdint>
#include <vector>

namespace pisa {

class forwardIndex {
  public:
    forwardIndex() = default;

    /// Build from per-document term lists.
    /// @param docTerms  docTerms[d] = list of term IDs for document d
    /// @param termCount total number of unique terms (defines the term-ID space)
    forwardIndex(const std::vector<std::vector<uint32_t>>& docTerms, std::size_t termCount)
        : m_termCount(termCount) {
        m_offsets.reserve(docTerms.size() + 1);
        m_offsets.push_back(0);
        for (const auto& terms : docTerms) {
            m_terms.insert(m_terms.end(), terms.begin(), terms.end());
            m_offsets.push_back(static_cast<uint32_t>(m_terms.size()));
        }
    }

    [[nodiscard]] std::size_t termCount() const { return m_termCount; }

    [[nodiscard]] std::vector<uint32_t> terms(uint32_t doc) const {
        return {m_terms.begin() + m_offsets[doc],
                m_terms.begin() + m_offsets[doc + 1]};
    }

  private:
    std::size_t m_termCount = 0;
    std::vector<uint32_t> m_terms;
    std::vector<uint32_t> m_offsets;
};

} // namespace pisa
