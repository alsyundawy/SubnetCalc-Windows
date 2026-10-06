#ifndef SUBNETCALC_ENGINE_CLOUD_H
#define SUBNETCALC_ENGINE_CLOUD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    CLOUD_PROFILE_STANDARD = 0,
    CLOUD_PROFILE_AWS,
    CLOUD_PROFILE_AZURE,
    CLOUD_PROFILE_GCP,
    CLOUD_PROFILE_OCI,
    CLOUD_PROFILE_COUNT
} cloud_profile_id_t;

typedef struct {
    uint32_t ip;
    const char *role;
} cloud_reserved_role_t;

const char *cloud_profile_name(cloud_profile_id_t profile);
int cloud_profile_min_prefix(cloud_profile_id_t profile);
int cloud_profile_reserved_count(cloud_profile_id_t profile, int prefix);
bool cloud_profile_usable_range(cloud_profile_id_t profile, uint32_t network, int prefix,
                                uint32_t *out_start, uint32_t *out_end, uint32_t *out_count);
size_t cloud_profile_reserved_roles(cloud_profile_id_t profile, uint32_t network, int prefix,
                                    cloud_reserved_role_t *roles, size_t max_roles);

#endif /* SUBNETCALC_ENGINE_CLOUD_H */
