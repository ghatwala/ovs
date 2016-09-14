/* Copyright (c) 2016 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#ifndef OFPROTO_DPIF_XLATE_CACHE_H
#define OFPROTO_DPIF_XLATE_CACHE_H 1

#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "openvswitch/types.h"
#include "dp-packet.h"
#include "odp-util.h"
#include "ofproto/ofproto-dpif-mirror.h"
#include "openvswitch/ofpbuf.h"

struct bfd;
struct bond;
struct dpif_flow_stats;
struct flow;
struct group_dpif;
struct mbridge;
struct netdev;
struct netflow;
struct ofpbuf;
struct ofproto_dpif;
struct ofputil_bucket;
struct ofputil_flow_mod;
struct rule_dpif;

enum xc_type {
    XC_TABLE,
    XC_RULE,
    XC_BOND,
    XC_NETDEV,
    XC_NETFLOW,
    XC_MIRROR,
    XC_LEARN,
    XC_NORMAL,
    XC_FIN_TIMEOUT,
    XC_GROUP,
    XC_TNL_NEIGH,
};

/* xlate_cache entries hold enough information to perform the side effects of
 * xlate_actions() for a rule, without needing to perform rule translation
 * from scratch. The primary usage of these is to submit statistics to objects
 * that a flow relates to, although they may be used for other effects as well
 * (for instance, refreshing hard timeouts for learned flows).
 *
 * An explicit reference is taken to all pointers other than the ones for
 * struct ofproto_dpif.  ofproto_dpif pointers are explicitly protected by
 * destroying all xlate caches before the ofproto is destroyed. */
struct xc_entry {
    enum xc_type type;
    union {
        struct {
            struct ofproto_dpif *ofproto;
            uint8_t id;
            bool    match; /* or miss. */
        } table;
        struct rule_dpif *rule;
        struct {
            struct netdev *tx;
            struct netdev *rx;
            struct bfd *bfd;
        } dev;
        struct {
            struct netflow *netflow;
            struct flow *flow;
            ofp_port_t iface;
        } nf;
        struct {
            struct mbridge *mbridge;
            mirror_mask_t mirrors;
        } mirror;
        struct {
            struct bond *bond;
            struct flow *flow;
            uint16_t vid;
        } bond;
        struct {
            struct ofproto_dpif *ofproto;
            struct ofputil_flow_mod *fm;
            struct ofpbuf *ofpacts;
        } learn;
        struct {
            struct ofproto_dpif *ofproto;
            ofp_port_t in_port;
            struct eth_addr dl_src;
            int vlan;
            bool is_gratuitous_arp;
        } normal;
        struct {
            struct rule_dpif *rule;
            uint16_t idle;
            uint16_t hard;
        } fin;
        struct {
            struct group_dpif *group;
            struct ofputil_bucket *bucket;
        } group;
        struct {
            char br_name[IFNAMSIZ];
            struct in6_addr d_ipv6;
        } tnl_neigh_cache;
    };
};

#define XC_ENTRY_FOR_EACH(ENTRY, ENTRIES)                       \
    for (ENTRY = ofpbuf_try_pull(ENTRIES, sizeof *ENTRY);       \
         ENTRY;                                                 \
         ENTRY = ofpbuf_try_pull(ENTRIES, sizeof *ENTRY))

struct xlate_cache {
    struct ofpbuf entries;
};

struct xlate_cache *xlate_cache_new(void);
struct xc_entry *xlate_cache_add_entry(struct xlate_cache *, enum xc_type);
void xlate_push_stats_entry(struct xc_entry *, const struct dpif_flow_stats *);
void xlate_push_stats(struct xlate_cache *, const struct dpif_flow_stats *);
void xlate_cache_clear_entry(struct xc_entry *);
void xlate_cache_clear(struct xlate_cache *);
void xlate_cache_delete(struct xlate_cache *);

#endif /* ofproto-dpif-xlate-cache.h */
