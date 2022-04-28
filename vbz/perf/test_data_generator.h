#pragma once

#include "../test/test_data.h"

#include <gsl/gsl-lite.hpp>

#include <numeric>
#include <random>

// Generator that targets a consistent number of bytes of increasing sequence.
template <typename T> struct SequenceGenerator
{
    static const std::size_t byte_target = 1000 * 1000; // 1mb
    
    static std::vector<std::vector<T>> generate(std::size_t& max_element_count)
    {
        std::vector<T> input_values(byte_target / sizeof(T));
        max_element_count = input_values.size();
        std::iota(input_values.begin(), input_values.end(), 0);

        return { input_values };
    }
};

// Generator that targets a random set of reads of random lengths.
//
// Under a maximum target byte count.
template <typename T>
struct SignalGenerator
{
    static const std::size_t byte_target = 100 * 1000 * 1000; // 100 mb

    static std::vector<std::vector<T>> generate(std::size_t& max_element_count)
    {
        auto const seed = 5;
        
        static std::size_t max_element_count_static;
        static auto const generated_reads = do_generation(seed, max_element_count_static);
        
        max_element_count = max_element_count_static;
        return generated_reads;
    }
    
private:
    static std::vector<std::vector<T>> do_generation(unsigned int seed, std::size_t& max_element_count)
    {
        std::random_device rd;  
        std::default_random_engine rand(seed);
        std::uniform_int_distribution<std::uint32_t> length_dist(30000, 200000);

        std::size_t generated_bytes = 0;
        std::vector<std::vector<T>> results;
        while (generated_bytes < byte_target)
        {
            auto length = std::min<std::size_t>((byte_target-generated_bytes)/sizeof(T), length_dist(rand));
            generated_bytes += length * sizeof(T);
            
            std::vector<T> input_values(length);
            max_element_count = std::max(max_element_count, input_values.size());


            std::size_t idx = 0;
            for (auto& e : input_values)
            {
                auto input_data = test_data[idx];
                e = (T)input_data;
                idx = (idx + 1) % test_data.size();
            }

            results.push_back(input_values);
        }

        return results;
    }
};
