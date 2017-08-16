/*
 * Copyright (C) 2016 Canonical, Ltd.
 * Author: Martin Pitt <martin.pitt@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <uuid.h>
#include <yaml.h>

/****************************************************
 * Parsed definitions
 ****************************************************/

typedef enum {
    ND_NONE,
    /* physical devices */
    ND_ETHERNET,
    ND_WIFI,
    /* virtual devices */
    ND_VIRTUAL,
    ND_BRIDGE = ND_VIRTUAL,
    ND_BOND,
    ND_VLAN,
} netdef_type;

typedef enum {
    BACKEND_NONE,
    BACKEND_NETWORKD,
    BACKEND_NM,
} netdef_backend;

typedef struct missing_node {
    char* netdef_id;
    yaml_node_t* node;
} missing_node;

/**
 * Represent a configuration stanza
 */
typedef struct net_definition {
    netdef_type type;
    netdef_backend backend;
    char* id;
    /* only necessary for NetworkManager connection UUIDs in some cases */
    uuid_t uuid;

    /* addresses */
    gboolean dhcp4;
    gboolean dhcp6;
    GArray* ip4_addresses;
    GArray* ip6_addresses;
    char* gateway4;
    char* gateway6;
    GArray* ip4_nameservers;
    GArray* ip6_nameservers;
    GArray* search_domains;
    GArray* routes;

    /* master ID for slave devices */
    char* bridge;
    char* bond;

    /* vlan */
    guint vlan_id;
    struct net_definition* vlan_link;
    gboolean has_vlans;

    /* Configured custom MAC address */
    char* set_mac;

    /* interface mtu */
    guint mtubytes;

    /* these properties are only valid for physical interfaces (type < ND_VIRTUAL) */
    char* set_name;
    struct {
        char* driver;
        char* mac;
        char* original_name;
    } match;
    gboolean has_match;
    gboolean wake_on_lan;

    /* these properties are only valid for ND_WIFI */
    GHashTable* access_points; /* SSID → wifi_access_point* */

    struct {
        char* mode;
        char* lacp_rate;
        guint monitor_interval;
        guint min_links;
        char* transmit_hash_policy;
        char* selection_logic;
        gboolean all_slaves_active;
        guint arp_interval;
        GArray* arp_ip_targets;
        char* arp_validate;
        char* arp_all_targets;
        guint up_delay;
        guint down_delay;
        char* fail_over_mac_policy;
        guint gratuitious_arp;
        /* TODO: unsolicited_na */
        guint packets_per_slave;
        char* primary_reselect_policy;
        guint resend_igmp;
        guint learn_interval;
        char* primary_slave;
    } bond_params;

    struct {
        guint ageing_time;
        guint priority;
        guint forward_delay;
        guint hello_time;
        guint max_age;
        guint path_cost;
        gboolean stp;
    } bridge_params;
    gboolean custom_bridging;

} net_definition;

typedef enum {
    WIFI_MODE_INFRASTRUCTURE,
    WIFI_MODE_ADHOC,
    WIFI_MODE_AP
} wifi_mode;

typedef struct {
    wifi_mode mode;
    char* ssid;
    char* password;
} wifi_access_point;

#define METRIC_UNSPEC G_MAXUINT

typedef struct {
    guint family;

    char* to;
    char* via;

    /* valid metrics are valid positive integers.
     * invalid metrics are represented by METRIC_UNSPEC */
    guint metric;
} ip_route;

/* Written/updated by parse_yaml(): char* id →  net_definition */
extern GHashTable* netdefs;

/****************************************************
 * Functions
 ****************************************************/

gboolean parse_yaml(const char* filename, GError** error);
gboolean finish_parse(GError** error);
netdef_backend get_global_backend();
