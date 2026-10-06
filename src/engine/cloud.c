#include "cloud.h"
#include "ipv4.h"

const char *cloud_profile_name(cloud_profile_id_t profile) {
    switch (profile) {
    case CLOUD_PROFILE_STANDARD:
        return "Standard (RFC 1918)";
    case CLOUD_PROFILE_AWS:
        return "AWS VPC";
    case CLOUD_PROFILE_AZURE:
        return "Azure VNet";
    case CLOUD_PROFILE_GCP:
        return "Google Cloud (GCP)";
    case CLOUD_PROFILE_OCI:
        return "Oracle Cloud (OCI)";
    default:
        return "Unknown";
    }
}

int cloud_profile_min_prefix(cloud_profile_id_t profile) {
    switch (profile) {
    case CLOUD_PROFILE_STANDARD:
        return 32;
    case CLOUD_PROFILE_AWS:
        return 28;
    case CLOUD_PROFILE_AZURE:
        return 29;
    case CLOUD_PROFILE_GCP:
        return 29;
    case CLOUD_PROFILE_OCI:
        return 30;
    default:
        return 32;
    }
}

int cloud_profile_reserved_count(cloud_profile_id_t profile, int prefix) {
    int min_pref = cloud_profile_min_prefix(profile);
    if (prefix > min_pref) {
        return 0;
    }
    switch (profile) {
    case CLOUD_PROFILE_STANDARD:
        return (prefix <= 30) ? 2 : 0;
    case CLOUD_PROFILE_AWS:
        return 5;
    case CLOUD_PROFILE_AZURE:
        return 5;
    case CLOUD_PROFILE_GCP:
        return 4;
    case CLOUD_PROFILE_OCI:
        return 3;
    default:
        return 0;
    }
}

bool cloud_profile_usable_range(cloud_profile_id_t profile, uint32_t network, int prefix,
                                uint32_t *out_start, uint32_t *out_end, uint32_t *out_count) {
    int min_pref = cloud_profile_min_prefix(profile);
    if (prefix < 1 || prefix > min_pref) {
        return false;
    }
    uint32_t total = (prefix == 32) ? 1 : (1U << (unsigned int)(32 - prefix));
    uint32_t bcast = network + total - 1;

    switch (profile) {
    case CLOUD_PROFILE_STANDARD:
        if (prefix == 31) {
            if (out_start)
                *out_start = network;
            if (out_end)
                *out_end = bcast;
            if (out_count)
                *out_count = 2;
        } else if (prefix == 32) {
            if (out_start)
                *out_start = network;
            if (out_end)
                *out_end = network;
            if (out_count)
                *out_count = 1;
        } else {
            if (out_start)
                *out_start = network + 1;
            if (out_end)
                *out_end = bcast - 1;
            if (out_count)
                *out_count = total - 2;
        }
        return true;

    case CLOUD_PROFILE_AWS:
    case CLOUD_PROFILE_AZURE:
        if (out_start)
            *out_start = network + 4;
        if (out_end)
            *out_end = bcast - 1;
        if (out_count)
            *out_count = total - 5;
        return true;

    case CLOUD_PROFILE_GCP:
        if (out_start)
            *out_start = network + 2;
        if (out_end)
            *out_end = bcast - 2;
        if (out_count)
            *out_count = total - 4;
        return true;

    case CLOUD_PROFILE_OCI:
        if (out_start)
            *out_start = network + 2;
        if (out_end)
            *out_end = bcast - 1;
        if (out_count)
            *out_count = total - 3;
        return true;

    default:
        return false;
    }
}

size_t cloud_profile_reserved_roles(cloud_profile_id_t profile, uint32_t network, int prefix,
                                    cloud_reserved_role_t *roles, size_t max_roles) {
    int min_pref = cloud_profile_min_prefix(profile);
    if (prefix < 1 || prefix > min_pref || !roles || max_roles == 0) {
        return 0;
    }
    uint32_t total = (prefix == 32) ? 1 : (1U << (unsigned int)(32 - prefix));
    uint32_t bcast = network + total - 1;

    size_t count = 0;
    switch (profile) {
    case CLOUD_PROFILE_STANDARD:
        if (prefix <= 30) {
            if (count < max_roles)
                roles[count++] = (cloud_reserved_role_t){network, "Network Address"};
            if (count < max_roles)
                roles[count++] = (cloud_reserved_role_t){bcast, "Broadcast Address"};
        }
        break;

    case CLOUD_PROFILE_AWS:
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network, "Network Address (VPC Block)"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 1, "VPC Router (Default Gateway)"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 2, "Amazon-Provided DNS"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 3, "Future Use (Reserved by AWS)"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){bcast, "Broadcast Address"};
        break;

    case CLOUD_PROFILE_AZURE:
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network, "Network Address"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 1, "Default Gateway"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 2, "Primary Azure DNS"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 3, "Secondary Azure DNS"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){bcast, "Broadcast Address"};
        break;

    case CLOUD_PROFILE_GCP:
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network, "Network Address"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 1, "Default Gateway"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){bcast - 1, "Future Use (Reserved by GCP)"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){bcast, "Broadcast Address"};
        break;

    case CLOUD_PROFILE_OCI:
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network, "Network Address"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){network + 1, "Default Gateway"};
        if (count < max_roles)
            roles[count++] = (cloud_reserved_role_t){bcast, "Broadcast Address"};
        break;

    default:
        break;
    }
    return count;
}
