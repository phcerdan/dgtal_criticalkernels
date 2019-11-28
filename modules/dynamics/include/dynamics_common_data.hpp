/* ********************************************************************
 * Copyright (C) 2020 Pablo Hernandez-Cerdan.
 *
 * This file is part of SGEXT: http://github.com/phcerdan/sgext.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * *******************************************************************/

#ifndef SG_DYNAMICS_COMMON_DATA_HPP
#define SG_DYNAMICS_COMMON_DATA_HPP

#include "array_utilities.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

namespace SG {
struct ParticleMaterial {
    double radius;
    double volume;
    double mass;
};
struct ParticleDynamicProperties {
    ArrayUtilities::Array3D vel;
    ArrayUtilities::Array3D acc;
};
struct Particle {
    size_t id;
    ArrayUtilities::Array3D pos;
    ParticleDynamicProperties dynamics;
    ParticleMaterial material;
};
inline bool operator<(const Particle &lhs, const Particle &rhs) {
    return lhs.id < rhs.id;
}
struct ParticleNeighbors {
    size_t particle_id;            ///< id of the particle
    std::vector<size_t> neighbors; /// ids of the neighbors of the particle

    ParticleNeighbors() = default;
    ParticleNeighbors(const size_t &id) : particle_id(id){};
    ParticleNeighbors(const size_t &id,
                      const std::vector<size_t> &input_neighbors)
            : particle_id(id), neighbors(input_neighbors){};
    ParticleNeighbors(const ParticleNeighbors &) = default;
    ParticleNeighbors(ParticleNeighbors &&) = default;
    ParticleNeighbors &operator=(const ParticleNeighbors &) = default;
    ParticleNeighbors &operator=(ParticleNeighbors &&) = default;
    ~ParticleNeighbors() = default;
};

struct ParticleNeighborsCollection {
    std::vector<ParticleNeighbors> collection;
};

/**
struct BondedPair {
    // id of bonded particles
    size_t id_a;
    size_t id_b;
};
inline bool operator<(const BondedPair& lhs, const BondedPair& rhs)
{
    if(lhs.id_a < rhs.id_a){
        return true;
    }

    if(lhs.id_a == rhs.id_a){
        if(lhs.id_b < rhs.id_b){
            return true;
        } else {
            return false;
        }
    }
    return false;
}
// *
// * Ensures id_a to be smaller than id_b in a bonded_pair.
// *
// * @param bonded_pair
//
void sort(BondedPair & bonded_pair) {
    if(bonded_pair.id_a > bonded_pair.id_b) {
        std::swap(bonded_pair.id_a, bonded_pair.id_b);
    }
};
*/

/**
 * Simplest ParticleCollection.
 * No multi-threaded, no GPU.
 * For optimizations, have a look at HOOMD ParticleData and friends.
 * That includes spatial optimizations (trees) and multi-threading,
 * including GPU acceleration.
 * NOTE(phcerdan): Too early to optimize, but it would be possible to copy
 * from HOOMD if needed.
 * The main drawback (but I might be wrong) is that HOOMD's ParticleData
 * seems to have a fixed num of particles, at least in MD.
 */
struct ParticleCollection {
    std::vector<Particle> particles;
    bool sorted = false;

    inline void sort() {
        std::sort(std::begin(particles), std::end(particles));
        sorted = true;
    };
    template <typename ForwardIt>
    inline ForwardIt binary_find(const ForwardIt &first,
                                 const ForwardIt &last,
                                 const size_t &id_value) const {
        assert(sorted &&
               "Particles not sorted before binary_find. Call sort() first");
        constexpr auto comp = [](const Particle &p, const size_t &value) {
            return p.id < value;
        };
        return std::lower_bound(first, last, id_value, comp);
        // return (it != last && !comp(*it, id_value) ) ? it : last;
    }

    inline std::vector<Particle>::iterator binary_find(const size_t &id_value) {
        return binary_find(std::begin(particles), std::end(particles),
                           id_value);
    }
    inline std::vector<Particle>::const_iterator
    binary_find(const size_t &id_value) const {
        return binary_find(std::cbegin(particles), std::cend(particles),
                           id_value);
    }

    inline std::pair<std::vector<Particle>::const_iterator, size_t>
    find_particle_and_index(const size_t &id_value) const {
        const auto it = binary_find(id_value);
        if (it != particles.end()) {
            return std::make_pair(it, std::distance(particles.begin(), it));
        } else {
            return std::make_pair(it, std::numeric_limits<size_t>::max());
        }
    }

    inline std::pair<std::vector<Particle>::iterator, size_t>
    find_particle_and_index(const size_t &id_value) {
        const auto it = binary_find(id_value);
        if (it != particles.end()) {
            return std::make_pair(it, std::distance(particles.begin(), it));
        } else {
            return std::make_pair(it, std::numeric_limits<size_t>::max());
        }
    }

    inline size_t find_index(const size_t &id_value) const {
        const auto it = binary_find(id_value);
        if (it != particles.end()) {
            return std::distance(particles.begin(), it);
        } else {
            return std::numeric_limits<size_t>::max();
        }
    }
    friend void print(const ParticleCollection &collection);
};
void print(const ParticleCollection &collection) {
    for (const auto &p : collection.particles) {
        std::cout << p.id << ": " << ArrayUtilities::to_string(p.pos)
                  << std::endl;
    }
}

/**
 * System is a catch all structure to perform simulations,
 * Classes might need a reference to it in the constructor.
 */
struct System {
    ParticleCollection all;            ///< all particles
    ParticleNeighborsCollection bonds; ///< fixed bonds between particles
    /** Dynamic neighbors per particle based on positions. */
    ParticleNeighborsCollection collision_neighbor_list;
};

} // namespace SG
#endif