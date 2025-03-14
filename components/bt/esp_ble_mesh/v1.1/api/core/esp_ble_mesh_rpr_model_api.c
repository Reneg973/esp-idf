/*
 * SPDX-FileCopyrightText: 2020-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>

#include "btc/btc_manage.h"

#include "btc_ble_mesh_rpr_model.h"
#include "esp_ble_mesh_rpr_model_api.h"

#if CONFIG_BLE_MESH_RPR_CLI
esp_err_t esp_ble_mesh_register_rpr_client_callback(esp_ble_mesh_rpr_client_cb_t callback)
{
    ESP_BLE_HOST_STATUS_CHECK(ESP_BLE_HOST_STATUS_ENABLED);

    return (btc_profile_cb_set(BTC_PID_RPR_CLIENT, callback) == 0 ? ESP_OK : ESP_FAIL);
}

static bool remote_prov_client_get_need_param(esp_ble_mesh_opcode_t opcode)
{
    switch (opcode) {
    case ESP_BLE_MESH_MODEL_OP_RPR_SCAN_START:
    case ESP_BLE_MESH_MODEL_OP_RPR_EXT_SCAN_START:
    case ESP_BLE_MESH_MODEL_OP_RPR_LINK_OPEN:
    case ESP_BLE_MESH_MODEL_OP_RPR_LINK_CLOSE:
        return true;
    default:
        return false;
    }
}

esp_err_t esp_ble_mesh_rpr_client_send(esp_ble_mesh_client_common_param_t *params,
                                       esp_ble_mesh_rpr_client_msg_t *msg)
{
    btc_ble_mesh_rpr_client_args_t arg = {0};
    btc_msg_t btc_msg = {0};

    if (params == NULL || params->model == NULL ||
        params->ctx.net_idx == ESP_BLE_MESH_KEY_UNUSED ||
        !ESP_BLE_MESH_ADDR_IS_UNICAST(params->ctx.addr) ||
        (remote_prov_client_get_need_param(params->opcode) && msg == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_BLE_HOST_STATUS_CHECK(ESP_BLE_HOST_STATUS_ENABLED);

    btc_msg.sig = BTC_SIG_API_CALL;
    btc_msg.pid = BTC_PID_RPR_CLIENT;
    btc_msg.act = BTC_BLE_MESH_ACT_RPR_CLIENT_SEND;

    arg.rpr_send.params = params;
    arg.rpr_send.msg = msg;

    return (btc_transfer_context(&btc_msg, &arg, sizeof(btc_ble_mesh_rpr_client_args_t),
            btc_ble_mesh_rpr_client_arg_deep_copy,
            btc_ble_mesh_rpr_client_arg_deep_free) == BT_STATUS_SUCCESS ? ESP_OK : ESP_FAIL);
}

esp_err_t esp_ble_mesh_rpr_client_action(esp_ble_mesh_rpr_client_act_type_t type,
                                         esp_ble_mesh_rpr_client_act_param_t *param)
{
    btc_ble_mesh_rpr_client_args_t arg = {0};
    btc_msg_t msg = {0};

    if (type > ESP_BLE_MESH_RPR_CLIENT_ACT_MAX || param == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_BLE_HOST_STATUS_CHECK(ESP_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_RPR_CLIENT;
    msg.act = BTC_BLE_MESH_ACT_RPR_CLIENT_ACT;

    arg.rpr_act.type = type;
    arg.rpr_act.param = param;

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_rpr_client_args_t),
            btc_ble_mesh_rpr_client_arg_deep_copy,
            btc_ble_mesh_rpr_client_arg_deep_free) == BT_STATUS_SUCCESS ? ESP_OK : ESP_FAIL);
}
#endif /* CONFIG_BLE_MESH_RPR_CLI */

#if CONFIG_BLE_MESH_RPR_SRV
esp_err_t esp_ble_mesh_register_rpr_server_callback(esp_ble_mesh_rpr_server_cb_t callback)
{
    ESP_BLE_HOST_STATUS_CHECK(ESP_BLE_HOST_STATUS_ENABLED);

    return (btc_profile_cb_set(BTC_PID_RPR_SERVER, callback) == 0 ? ESP_OK : ESP_FAIL);
}

esp_err_t esp_ble_mesh_rpr_server_set_uuid_match(const uint8_t *match_val, uint8_t match_len, uint8_t offset)
{
    btc_ble_mesh_rpr_server_args_t arg = {0};
    btc_msg_t msg = {0};

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_RPR_SERVER;
    msg.act = BTC_BLE_MESH_ACT_RPR_SRV_SET_UUID_MATCH;

    if (!match_len || match_len > 16 ||
        !match_val || offset >= 16 ||
        offset + match_len > 16) {
        return ESP_ERR_INVALID_ARG;
    }

    arg.set_uuid_match.match_val = bt_mesh_calloc(match_len);
    if (!arg.set_uuid_match.match_val) {
        BT_ERR("%s:Out of memory", __func__);
        return ESP_ERR_NO_MEM;
    }

    memcpy(arg.set_uuid_match.match_val, match_val, match_len);
    arg.set_uuid_match.match_len = match_len;
    arg.set_uuid_match.offset = offset;

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_rpr_server_args_t),
                                 btc_ble_mesh_rpr_server_arg_deep_copy,
                                 btc_ble_mesh_rpr_server_arg_deep_free) == BT_STATUS_SUCCESS ? ESP_OK : ESP_FAIL);
}
#endif /* CONFIG_BLE_MESH_RPR_SRV */
