#ifndef __EVALRESP_XML_TO_CHAR_H__
#define __EVALRESP_XML_TO_CHAR_H__

#include <evalresp_log/log.h>

/**
 * @private
 * @param[in] log logging structure where you want information to be sent
 * @param[in] xml_in char * containing xml to be translated
 * @param[in,out] resp_out a pointer where to allocate and store the translated response file as char *
 * @retval EVALRESP_OK on succes
 * @pre xml_in must not be NULL and *resp_out must be NULL
 * @post *resp_out will be allocated using calloc that must be freed
 * @brief house keeping function to initiate xml -> resp files but stored as char *
 */
int evalresp_convert_xml_to_char (evalresp_log_t *log, char *xml_in, char **resp_out);

/**
 * @private
 * @param[in] log logging structure where you want information to be sent
 * @param[in] xml_in char * containing xml to be translated
 * @param[out] root the root node of the xml converted to c stuructures
 * @retval EVALRESP_OK on succes
 * @sa x2r_fdsn_station_xml
 * @pre xml_in must not be NULL
 * @post root must be freed
 * @brief load the xml string into a mxml generated structure
 */
int evalresp_load_mxml_service (evalresp_log_t *log, char *xml_in, x2r_fdsn_station_xml **root);
/**
 * @private
 * @param[in] log logging structure where you want information to be sent
 * @param[in] root the root node of the xml converted to c stuructures
 * @param[in,out] resp_out a pointer where to allocate and store the translated response file as char *
 * @retval EVALRESP_OK on succes
 * @post *resp_out will be allocated using calloc that must be freed
 * @brief convert mxml generated structure to a response file storred as char *
 */
int evalresp_save_mxml_service_to_char (evalresp_log_t *log, x2r_fdsn_station_xml *root, char **resp_out);

#endif /*__EVALRESP_XML_TO_CHAR_H__*/
