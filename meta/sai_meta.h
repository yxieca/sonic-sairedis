#ifndef __SAI_META_H__
#define __SAI_META_H__

extern "C" {
#include "sai.h"
#include "saimetadata.h"
}

#define MAX_LIST_COUNT 0x1000

#define DEFAULT_VLAN_NUMBER 1
#define MINIMUM_VLAN_NUMBER 1
#define MAXIMUM_VLAN_NUMBER 4094

extern void dump_object_reference();
extern sai_status_t meta_init_db();

// GENERIC FUNCTION POINTERS

typedef sai_status_t (*sai_create_generic_fn)(
        _In_ sai_object_type_t object_type,
        _Out_ sai_object_id_t* object_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list);

typedef sai_status_t (*sai_remove_generic_fn)(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id);

typedef sai_status_t (*sai_set_generic_attribute_fn)(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ const sai_attribute_t *attr);

typedef sai_status_t (*sai_get_generic_attribute_fn)(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list);

// META GENERIC

extern sai_status_t meta_sai_create_oid(
        _In_ sai_object_type_t object_type,
        _Out_ sai_object_id_t* object_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list,
        _In_ sai_create_generic_fn create);

extern sai_status_t meta_sai_remove_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ sai_remove_generic_fn remove);

extern sai_status_t meta_sai_set_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ const sai_attribute_t *attr,
        _In_ sai_set_generic_attribute_fn set);

extern sai_status_t meta_sai_get_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list,
        _In_ sai_get_generic_attribute_fn get);

// META ENTRY QUAD

#define META_CREATE_ENTRY(ot)                               \
    extern sai_status_t meta_sai_create_ ## ot(             \
            _In_ const sai_ ## ot ## _t* ot,                \
            _In_ uint32_t attr_count,                       \
            _In_ const sai_attribute_t *attr_list,          \
            _In_ sai_create_ ## ot ## _fn create);

#define META_REMOVE_ENTRY(ot)                               \
    extern sai_status_t meta_sai_remove_ ## ot(             \
            _In_ const sai_ ## ot ## _t* ot,                \
            _In_ sai_remove_ ## ot ##_fn remove);

#define META_SET_ENTRY(ot)                                  \
    extern sai_status_t meta_sai_set_ ## ot(                \
            _In_ const sai_ ## ot ## _t* ot,                \
            _In_ const sai_attribute_t *attr,               \
            _In_ sai_set_ ## ot ## _attribute_fn set);

#define META_GET_ENTRY(ot)                                  \
    extern sai_status_t meta_sai_get_ ## ot(                \
            _In_ const sai_ ## ot ## _t* ot,                \
            _In_ uint32_t attr_count,                       \
            _Inout_ sai_attribute_t *attr_list,             \
            _In_ sai_get_ ## ot ## _attribute_fn get);

#define META_QUAD_ENTRY(ot)     \
    META_CREATE_ENTRY(ot);      \
    META_REMOVE_ENTRY(ot);      \
    META_SET_ENTRY(ot);         \
    META_GET_ENTRY(ot)

META_QUAD_ENTRY(fdb_entry);
META_QUAD_ENTRY(inseg_entry);
META_QUAD_ENTRY(ipmc_entry);
META_QUAD_ENTRY(l2mc_entry);
META_QUAD_ENTRY(mcast_fdb_entry);
META_QUAD_ENTRY(neighbor_entry);
META_QUAD_ENTRY(route_entry);

// STATS

typedef sai_status_t (*sai_get_generic_stats_fn)(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ const sai_enum_metadata_t *enum_metadata,
        _In_ uint32_t number_of_counters,
        _In_ const int32_t *counter_ids,
        _Out_ uint64_t *counters);

sai_status_t meta_sai_get_stats_oid(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id,
        _In_ const sai_enum_metadata_t* stats_enum,
        _In_ uint32_t count,
        _In_ const int32_t *counter_id_list,
        _Out_ uint64_t *counter_list,
        _In_ sai_get_generic_stats_fn get_stats);

// NOTIFICATIONS

extern void meta_sai_on_fdb_event(
        _In_ uint32_t count,
        _In_ sai_fdb_event_notification_data_t *data);

// FDB FLUSH

extern sai_status_t meta_sai_flush_fdb_entries(
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list,
        _In_ sai_flush_fdb_entries_fn flush_fdb_entries);

// UNIT TESTS HELPERS

/**
 * @brief Enable unittest globally.
 *
 * @param[in] enable If set to true unittests are enabled.
 */
void meta_unittests_enable(
        _In_ bool enable);

/**
 * @brief Indicates whethre unittests are enabled;
 */
bool meta_unittests_enabled();

/**
 * @bried Allow to perform SET operation on READ_ONLY attribue only once.
 *
 * This function relaxes metadata checking on SET operation, it allows to
 * perform SET api on READ_ONLY attribute only once on specific object type and
 * specific attribue.
 *
 * Once means that SET operation is only relaxed for the very next SET call on
 * that specific object type and attrirbute id.
 *
 * Function is explicitly named ONCE, since it will force test developer to not
 * forget that SET check is relaxed, and not forget for future unittests.
 *
 * Function is provided for more flexible testing using virtual switch.  Since
 * some of the read only attributes maybe very complex to simulate (for example
 * resources used by actual asic when adding next hop or next hop group), then
 * it's easier to write such unittest:
 *
 * TestCase:
 * 1. meta_unittests_allow_readonly_set_once(x,y);
 * 2. object_x_api->set_attribyte(object_id, attr, foo); // attr.id == y
 * 3. object_x_api->get_attribute(object_id, 1, attr); // attr.id == y
 * 4. check if get result is equal to set result.
 *
 * On real ASIC, even after allowing SET on read only attribute, actual SET
 * should fail.
 *
 * It can be dangerous to set any readonly attribute to different values since
 * internal metadata logic maybe using that value and in some cases metadata
 * database may get out of sync and cause unexpected results in api calls up to
 * application carash.
 *
 * This function is not thread safe.
 *
 * @param[in] object_type Object type on which SET will be possible.
 * @param[in] attr_id Attribute ID on which SET will be possible.
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t meta_unittests_allow_readonly_set_once(
        _In_ sai_object_type_t object_type,
        _In_ int32_t attr_id);

// POST VALIDATE

/*
 * Those functions will be used to recreate virtual switch local metadata state
 * after WARM BOOT.
 */

void meta_warm_boot_notify();

void meta_generic_validation_post_create(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ sai_object_id_t switch_id,
        _In_ const uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list);

void meta_generic_validation_post_set(
        _In_ const sai_object_meta_key_t& meta_key,
        _In_ const sai_attribute_t *attr);

#endif // __SAI_META_H__
