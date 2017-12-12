#include "sai_redis.h"
#include "meta/sai_serialize.h"
#include "meta/saiattributelist.h"

sai_status_t internal_redis_generic_remove(
        _In_ sai_object_type_t object_type,
        _In_ const std::string &serialized_object_id)
{
    SWSS_LOG_ENTER();

    std::string str_object_type = sai_serialize_object_type(object_type);

    std::string key = str_object_type + ":" + serialized_object_id;

    SWSS_LOG_DEBUG("generic remove key: %s", key.c_str());

    if (g_record)
    {
        recordLine("r|" + key);
    }

    g_asicState->del(key, "remove");

    return SAI_STATUS_SUCCESS;
}

sai_status_t redis_generic_remove(
        _In_ sai_object_type_t object_type,
        _In_ sai_object_id_t object_id)
{
    SWSS_LOG_ENTER();

    std::string str_object_id = sai_serialize_object_id(object_id);

    sai_status_t status = internal_redis_generic_remove(
            object_type,
            str_object_id);

    if (object_type == SAI_OBJECT_TYPE_SWITCH &&
            status == SAI_STATUS_SUCCESS)
    {
        SWSS_LOG_NOTICE("removing switch id %s", sai_serialize_object_id(object_id).c_str());

        redis_free_virtual_object_id(object_id);

        // TODO do we need some more actions here ? to clean all
        // objects that are in the same switch that were snooped
        // inside metadata ? should that be metadata job?
    }

    return status;
}

sai_status_t redis_bulk_generic_remove(
        _In_ sai_object_type_t object_type,
        _In_ uint32_t object_count,
        _In_ const sai_object_id_t *object_id, /* array */
        _Out_ sai_status_t *object_statuses) /* array */
{
    SWSS_LOG_ENTER();

    std::vector<std::string> serialized_object_ids;

    // on create vid is put in db by syncd
    for (uint32_t idx = 0; idx < object_count; idx++)
    {
        std::string str_object_id = sai_serialize_object_id(object_id[idx]);
        serialized_object_ids.push_back(str_object_id);
    }

    return internal_redis_bulk_generic_remove(
            object_type,
            serialized_object_ids,
            object_statuses);
}

sai_status_t internal_redis_bulk_generic_remove(
        _In_ sai_object_type_t object_type,
        _In_ const std::vector<std::string> &serialized_object_ids,
        _Out_ sai_status_t *object_statuses) /* array */
{
    SWSS_LOG_ENTER();

    std::string str_object_type = sai_serialize_object_type(object_type);

    std::vector<swss::FieldValueTuple> entries;

    /*
     * We are recording all entries and their statuses, but we send to sairedis
     * only those that succeeded metadata check, since only those will be
     * executed on syncd, so there is no need with bothering decoding statuses
     * on syncd side.
     */

    for (size_t idx = 0; idx < serialized_object_ids.size(); ++idx)
    {
        std::string str_attr = "";

        swss::FieldValueTuple fvtNoStatus(serialized_object_ids[idx], str_attr);

        entries.push_back(fvtNoStatus);
    }

    /*
     * We are adding number of entries to actualy add ':' to be compatible
     * with previous
     */

    if (g_record)
    {
        std::string joined;

        for (const auto &e: entries)
        {
            // ||obj_id|attr=val|attr=val||obj_id|attr=val|attr=val

            joined += "||" + fvField(e) + "|" + fvValue(e);
        }

        /*
         * Capital 'C' stads for bulk CREATE operation.
         */

        recordLine("C|" + str_object_type + joined);
    }

    // key:         object_type:count
    // field:       object_id
    // value:       object_attrs
    std::string key = str_object_type + ":" + std::to_string(entries.size());

    if (entries.size())
    {
        g_asicState->set(key, entries, "bulkremove");
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t redis_generic_remove_fdb_entry(
        _In_ const sai_fdb_entry_t* fdb_entry)
{
    SWSS_LOG_ENTER();

    std::string str_fdb_entry = sai_serialize_fdb_entry(*fdb_entry);

    return internal_redis_generic_remove(
            SAI_OBJECT_TYPE_FDB_ENTRY,
            str_fdb_entry);
}

sai_status_t redis_generic_remove_neighbor_entry(
        _In_ const sai_neighbor_entry_t* neighbor_entry)
{
    SWSS_LOG_ENTER();

    std::string str_neighbor_entry = sai_serialize_neighbor_entry(*neighbor_entry);

    return internal_redis_generic_remove(
            SAI_OBJECT_TYPE_NEIGHBOR_ENTRY,
            str_neighbor_entry);
}

sai_status_t redis_generic_remove_route_entry(
        _In_ const sai_route_entry_t* route_entry)
{
    SWSS_LOG_ENTER();

    std::string str_route_entry = sai_serialize_route_entry(*route_entry);

    return internal_redis_generic_remove(
            SAI_OBJECT_TYPE_ROUTE_ENTRY,
            str_route_entry);
}
