#ifndef MADNESS_INDEXIT_H
#define MADNESS_INDEXIT_H

/// \file indexit.h
/// \brief Provides IndexIterator

#include <vector>
#include <world/print.h>

namespace madness {

    /// Facilitates iteration through a multidimension index space
    class IndexIterator {
    private:
        std::vector<long> n; ///< User specified upper limits for each dimension
        std::vector<long> i; ///< Current index
        bool finished;
        
    public:
        template <typename V>
        IndexIterator(const V& limits) : n(limits.size()), i(limits.size(),0), finished(false) 
        {
            for (unsigned int d=0; d<n.size(); d++) n[d] = limits[d];
        }

        IndexIterator(int ndim, const long limits[]) : n(ndim), i(ndim,0), finished(false)
        {
            for (unsigned int d=0; d<n.size(); d++) n[d] = limits[d];
        }
            
        
        long operator[](int d) const {MADNESS_ASSERT(!finished); return i[d];}
        
        const std::vector<long>& operator*() const {MADNESS_ASSERT(!finished); return i;}
        
        operator bool() const {return !finished;}
        
        IndexIterator& operator++() {
            for (int d=n.size()-1; d>=0; d--) {
                i[d]++;
                if (i[d] < n[d]) return *this;
                else i[d] = 0;
            }
            finished = true;
            return *this;
        }

        static void test() {
            Vector<int,4> n(3);
            for (IndexIterator it(n); it; ++it) {
                print(*it);
            }
        }
    };

}

#endif