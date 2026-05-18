#include <iostream>
#include <vector>
#include <utility>

/**
 * @brief Surcharge de l'opérateur << pour afficher facilement n'importe quel std::vector.
 */
template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec)
{
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        os << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    os << "]";
    return os;
}

/**
 * @brief Surcharge de l'opérateur << pour afficher facilement n'importe quel std::pair.
 */
template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p)
{
    return os << "(" << p.first << ", " << p.second << ")";
}