#pragma once

#include <libasn/ber.h>

namespace libasn {
namespace ldap {

enum class result_code_enum {
    SUCCESS                        = 0,
    OPERATIONS_ERROR               = 1,
    PROTOCOL_ERROR                 = 2,
    TIME_LIMIT_EXCEEDED            = 3,
    SIZE_LIMIT_EXCEEDED            = 4,
    COMPARE_FALSE                  = 5,
    COMPARE_TRUE                   = 6,
    AUTH_METHOD_NOT_SUPPORTED      = 7,
    STRONGER_AUTH_REQUIRED         = 8,
    REFERRAL                       = 10,
    ADMIN_LIMIT_EXCEEDED           = 11,
    UNAVAILABLE_CRITICAL_EXTENSION = 12,
    CONFIDENTIALITY_REQUIRED       = 13,
    SASL_BIND_IN_PROGRESS          = 14,
    NO_SUCH_ATTRIBUTE              = 16,
    UNDEFINED_ATTRIBUTE_TYPE       = 17,
    INAPPROPRIATE_MATCHING         = 18,
    CONSTRAINT_VIOLATION           = 19,
    ATTRIBUTE_VALUE_EXISTS         = 20,
    INVALID_ATTRIBUTE_SYNTAX       = 21,
    NO_SUCH_OBJECT                 = 32,
    ALIAS_PROBLEM                  = 33,
    INVALID_DN_SYNTAX              = 34,
    ALIAS_DEREFERENCING_PROBLEM    = 36,
    INAPPROPRIATE_AUTHENTICATION   = 48,
    INVALID_CREDENTIALS            = 49,
    INSUFFICIENT_ACCESS_RIGHTS     = 50,
    BUSY                           = 51,
    UNAVAILABLE                    = 52,
    IUNWILLING_TO_PERFORM          = 53,
    LOOP_DETECT                    = 54,
    NAMING_VIOLATION               = 64,
    OBJECT_CLASS_VIOLATION         = 65,
    NTO_ALLOWED_ON_NON_LEAF        = 66,
    NOT_ALLOEW_ON_RDN              = 67,
    ENTRY_ALREADY_EXISTS           = 68,
    OBJECT_CLASS_MODS_PROHIBITED   = 69,
    AFFECTS_MULTIPLE_DSAS          = 71,
    OTHER                          = 80
};

enum class protocol_op_enum {
    BIND_REQUEST            = 0,
    BIND_RESPONSE           = 1,
    UNBIND_REQUEST          = 2,
    SEARCH_REQUEST          = 3,
    SEARCH_RESULT_ENTRY     = 4,
    SEARCH_RESULT_DONE      = 5,
    SEARCH_RESULT_REFERENCE = 19,
    MODIFY_REQUEST          = 6,
    MODIFY_RESPONSE         = 7,
    ADD_REQUEST             = 8,
    ADD_RESPONSE            = 9,
    DEL_REQUEST             = 10,
    DEL_RESPONSE            = 11,
    MODIFY_DN_REQUEST       = 12,
    MODIFY_DN_RESPONSE      = 13,
    COMPARE_REQUEST         = 14,
    COMPARE_RESPONSE        = 15,
    ABANDON_REQUEST         = 16,
    EXTENDED_REQUEST        = 23,
    EXTENDED_RESPONSE       = 24,
    INTERMEDIATE_RESPONSE   = 25
};

constexpr auto message_id                 = ber::integer;
constexpr auto ldap_string                = ber::octet_string;
constexpr auto ldap_oid                   = ber::octet_string;
constexpr auto ldap_dn                    = ber::octet_string;
constexpr auto ldap_url                   = ldap_string;
constexpr auto relative_ldap_dn           = ldap_string;
constexpr auto attribute_type             = ldap_string;
constexpr auto attribute_description      = ldap_string;
constexpr auto attribute_description_list = ber::sequence_of(attribute_description);
constexpr auto attribute_value            = ber::octet_string;
constexpr auto attribute_value_assertion  = ber::sequence(attribute_description, attribute_value);
constexpr auto assertion_value            = ber::octet_string;
constexpr auto attribute                  = ber::sequence(attribute_description, ber::set_of(attribute_value));
constexpr auto matching_rule_id           = ldap_string;
constexpr auto referral                   = ber::sequence_of(ldap_url);
constexpr auto control                    = ber::sequence(ldap_oid, ber::boolean, ber::octet_string);
constexpr auto sasl_credentials           = ber::sequence(ldap_string, ber::optional(ldap_string));
constexpr auto attribute_type_and_values  = ber::sequence(attribute_description, ber::set_of(attribute_value));
constexpr auto ldap_result                = ber::sequence(ber::enumerated<result_code_enum>(), ldap_dn, ldap_string,
                                                          ber::optional(referral.context_specific<3>()));

constexpr auto matching_rule_assertion = ber::sequence(
    ber::optional(matching_rule_id.context_specific<1>()), ber::optional(attribute_description.context_specific<2>()),
    assertion_value.context_specific<3>(), ber::optional(ber::boolean.context_specific<4>()));

enum class authentification_choice_type_enum {
    SIMPLE = 0,
    SASL   = 3,
};

constexpr auto authentication_choice = ber::choice<authentification_choice_type_enum>()
                                           .with<authentification_choice_type_enum::SIMPLE>(ber::octet_string)
                                           .with<authentification_choice_type_enum::SASL>(sasl_credentials);

enum class substring_filter_substrings_enum {
    INITIAL = 0,
    ANY     = 1,
    FINAL   = 2,
};

constexpr auto substring = ber::choice<substring_filter_substrings_enum>()
                               .with<substring_filter_substrings_enum::INITIAL>(assertion_value)
                               .with<substring_filter_substrings_enum::ANY>(assertion_value)
                               .with<substring_filter_substrings_enum::FINAL>(assertion_value);

constexpr auto substring_filter = ber::sequence(attribute_description, ber::sequence_of(substring));

enum class filter_enum {
    AND              = 0,
    OR               = 1,
    NOT              = 2,
    EQUALITY_MATCH   = 3,
    SUBSTRINGS       = 4,
    GREATER_OR_EQUAL = 5,
    LESS_OR_EQUAL    = 6,
    PRESENT          = 7,
    APPROX_MATCH     = 8,
    EXTENSIBLE_MATCH = 9,
};

constexpr auto filter_logic = ber::choice<filter_enum>()
                                  .with<filter_enum::EQUALITY_MATCH>(attribute_value_assertion)
                                  .with<filter_enum::SUBSTRINGS>(substring_filter)
                                  .with<filter_enum::GREATER_OR_EQUAL>(attribute_value_assertion)
                                  .with<filter_enum::LESS_OR_EQUAL>(attribute_value_assertion)
                                  .with<filter_enum::PRESENT>(attribute_description)
                                  .with<filter_enum::APPROX_MATCH>(attribute_value_assertion)
                                  .with<filter_enum::EXTENSIBLE_MATCH>(matching_rule_assertion);

constexpr auto filter = ber::choice<filter_enum>()
                            .with<filter_enum::AND>(ber::set_of(filter_logic))
                            .with<filter_enum::OR>(ber::set_of(filter_logic))
                            .with<filter_enum::NOT>(ber::explicit_(filter_logic))
                            .with<filter_enum::EQUALITY_MATCH>(attribute_value_assertion)
                            .with<filter_enum::SUBSTRINGS>(substring_filter)
                            .with<filter_enum::GREATER_OR_EQUAL>(attribute_value_assertion)
                            .with<filter_enum::LESS_OR_EQUAL>(attribute_value_assertion)
                            .with<filter_enum::PRESENT>(attribute_description)
                            .with<filter_enum::APPROX_MATCH>(attribute_value_assertion)
                            .with<filter_enum::EXTENSIBLE_MATCH>(matching_rule_assertion);

constexpr auto attribute_selection = ber::sequence_of(ldap_string);

enum class search_request_scope_enum {
    BASE_OBJECT   = 0,
    SINGLE_LEVEL  = 1,
    WHOLE_SUBTREE = 2,
};
enum class search_request_deref_aliases_enum {
    NEVER_DEREF_ALIASES = 0,
    DEREF_IN_SEARCHING  = 1,
    DEREF_FIND_BASE_OBJ = 2,
    DEREF_ALWAYS        = 3,
};

constexpr auto partial_attribute      = ber::sequence(attribute_description, ber::set_of(attribute_value));
constexpr auto partial_attribute_list = ber::sequence_of(partial_attribute);
constexpr auto search_result_entry =
    ber::sequence(ldap_dn, partial_attribute_list).application<protocol_op_enum::SEARCH_RESULT_ENTRY>();

constexpr auto controls = ber::sequence_of(control).context_specific<0>();

enum class operation_enum {
    ADD     = 0,
    DELETE  = 1,
    REPLACE = 2,
};

/* Requests */
constexpr auto bind_request =
    ber::sequence(ber::integer, ldap_dn, authentication_choice).application<protocol_op_enum::BIND_REQUEST>();

constexpr auto search_request = ber::sequence(ldap_dn, ber::enumerated<search_request_scope_enum>(),
                                              ber::enumerated<search_request_deref_aliases_enum>(), ber::integer,
                                              ber::integer, ber::boolean, filter, attribute_selection)
                                    .application<protocol_op_enum::SEARCH_REQUEST>();

constexpr auto unbind_request = ber::null.application<protocol_op_enum::UNBIND_REQUEST>();

constexpr auto extended_request =
    ber::sequence(ldap_oid.context_specific<0>(), ber::optional(ber::octet_string.context_specific<1>()))
        .template application<protocol_op_enum::EXTENDED_REQUEST>();

constexpr auto abandon_request = message_id.application<protocol_op_enum::ABANDON_REQUEST>();

constexpr auto del_request = ldap_dn.application<protocol_op_enum::DEL_REQUEST>();

constexpr auto change = ber::sequence(ber::enumerated<operation_enum>(), partial_attribute_list);

constexpr auto modify_request =
    ber::sequence(ldap_dn, ber::sequence_of(change)).application<protocol_op_enum::MODIFY_REQUEST>();

constexpr auto compare_request =
    ber::sequence(ldap_dn, attribute_value_assertion).application<protocol_op_enum::COMPARE_REQUEST>();

constexpr auto modify_dn_request =
    ber::sequence(ldap_dn, relative_ldap_dn, ber::boolean, ber::optional(ldap_dn.context_specific<0>()))
        .application<protocol_op_enum::MODIFY_DN_REQUEST>();

/* Responses */
constexpr auto extended_response = ldap_result.application<protocol_op_enum::EXTENDED_RESPONSE>();

constexpr auto compare_response = ldap_result.application<protocol_op_enum::COMPARE_RESPONSE>();

constexpr auto intermediate_response =
    ber::sequence(ber::optional(ldap_oid.context_specific<0>()), ber::optional(ber::octet_string.context_specific<1>()))
        .application<protocol_op_enum::INTERMEDIATE_RESPONSE>();

constexpr auto bind_response = ldap_result.application<protocol_op_enum::BIND_RESPONSE>();

constexpr auto modify_response = ldap_result.application<protocol_op_enum::MODIFY_RESPONSE>();

constexpr auto modify_dn_response = ldap_result.application<protocol_op_enum::MODIFY_DN_RESPONSE>();

constexpr auto del_response = ldap_result.application<protocol_op_enum::DEL_RESPONSE>();

constexpr auto add_response = ldap_result.application<protocol_op_enum::ADD_RESPONSE>();

constexpr auto search_result_done = ldap_result.application<protocol_op_enum::SEARCH_RESULT_DONE>();

constexpr auto search_result_reference =
    ber::sequence_of(ldap_url).application<protocol_op_enum::SEARCH_RESULT_REFERENCE>();

constexpr auto message = ber::sequence(message_id,
                                       ber::choice<protocol_op_enum>()
                                           .with(bind_request)
                                           .with(unbind_request)
                                           .with(search_request)
                                           .with(extended_request)
                                           .with(abandon_request)
                                           .with(modify_request)
                                           .with(compare_request)
                                           .with(modify_dn_request)
                                           .with(del_request)

                                           .with(search_result_entry)
                                           .with(search_result_done)
                                           .with(bind_response)
                                           .with(extended_response)
                                           .with(compare_response)
                                           .with(intermediate_response)
                                           .with(modify_response)
                                           .with(modify_dn_response)
                                           .with(del_response)
                                           .with(add_response)
                                           .with(search_result_done)
                                           .with(search_result_reference),
                                       ber::optional(controls));
}  // namespace ldap
}  // namespace libasn
