// #include "traffic/traffic_data.hpp"

#pragma once

#include "common/types.hpp"
#include "traffic/demand_base.hpp"

#include <vector>
#include <string>

/**
 * @brief Contient l'ensemble des demandes de trafic d'une instance.
 *
 * Stocke les métadonnées des demandes (source, target, norme) et leurs volumes
 * sur chaque time slot. Les volumes sont aplatis en row-major (demand-major)
 * pour garantir la contiguïté mémoire des volumes d'une même demande.
 *
 * @note Layout mémoire : all_volumes_[demand_id * num_time_slots_ + t]
 */
class TrafficData
{
public:
    /**
     * @brief Construit un TrafficData à partir de demandes et de volumes pré-calculés.
     *
     * @param num_slots Nombre de time slots de l'instance.
     * @param demands   Vecteur des métadonnées de demandes (taille = n_demands).
     * @param volumes   Vecteur aplati des volumes (taille = n_demands * num_slots).
     *
     * @note Le constructeur appelle compute_all_norms_() pour remplir le champ n2
     *       de chaque DemandBase.
     */
    TrafficData(TickCount num_slots,
                const std::vector<DemandBase> &demands,
                const std::vector<double> &volumes);

    /**
     * @brief Constructeur par défaut. Crée un TrafficData vide.
     */
    TrafficData() = default;

    /**
     * @brief Renvoie le nombre de demandes.
     */
    uint16_t demands_count() const { return static_cast<uint16_t>(info_.size()); }

    /**
     * @brief Renvoie le nombre de time slots de l'instance.
     */
    TickCount slots_count() const { return num_time_slots_; }

    /**
     * @brief Renvoie le volume d'une demande à un time slot donné.
     *
     * @param demand_id Identifiant de la demande.
     * @param t         Time slot considéré.
     * @return Volume de la demande au slot t.
     */
    double volume(uint16_t demand_id, Tick t) const
    {
        return all_volumes_[demand_id * num_time_slots_ + t];
    }

    /**
     * @brief Renvoie les métadonnées statiques d'une demande (source, target, norme).
     *
     * @param demand_id Identifiant de la demande.
     */
    const DemandBase &get_info(uint16_t demand_id) const { return info_[demand_id]; }

    /**
     * @brief Charge un TrafficData depuis le fichier JSON d'une instance.
     *
     * Va lire `instances/<dataset>/<dataset>-<instance_id>-tm.json`.
     *
     * @param dataset      Nom du dataset (par exemple "setA").
     * @param instance_id  Identifiant de l'instance (par exemple "07").
     * @throws std::runtime_error Si le fichier ne peut pas être ouvert.
     */
    static TrafficData load(const std::string &dataset, const std::string &instance_id);

    /**
     * @brief Surcharge l'affichage du TrafficData (tableau Markdown des 5 premières demandes).
     */
    friend std::ostream &operator<<(std::ostream &os, const TrafficData &td);

private:
    std::vector<DemandBase> info_;    ///< Métadonnées des demandes (n_demands éléments).
    std::vector<double> all_volumes_; ///< Volumes aplatis row-major (n_demands * num_time_slots_).
    TickCount num_time_slots_;        ///< Nombre de time slots.

    /**
     * @brief Calcule la norme L2 des volumes pour chaque demande et la stocke dans info_[i].n2.
     *
     * Appelée automatiquement par le constructeur et par load().
     */
    void compute_all_norms_();
};