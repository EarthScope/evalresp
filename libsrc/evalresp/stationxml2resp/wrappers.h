#ifndef __EVALRESP_X2R_WRAPPERS_H__
#define __EVALRESP_X2R_WRAPPERS_H__

#include <evalresp_log/log.h>
#include <stdio.h>

/**
 * @param[in] log logging structure where you want information to be sent
 * @param[in] xml_flag if set to one then conversion happens otherwise nothing happens
 * @param[in] xml_in char * containing xml to be translated
 * @param[in,out] resp_out a pointer where to allocate and store the translated response file as char *
 * @retval EVALRESP_OK on success
 * @pre xml_in must not be NULL and *resp_out must be NULL if xml_flag is one
 * @post *resp_out will be allocated using calloc that must be freed if xml_flag is one
 * @brief do conversion of xml -> resp files but stored as char *
 */
int evalresp_xml_to_char (evalresp_logger *log, int xml_flag, char *xml_in, char **resp_out);

/**
 * @param[in] log logging structure where you want information to be sent
 * @param[in] xml_flag if set to one then conversion happens otherwise nothing happens
 * @param[in] xml_fd stream containing xml information
 * @parma[in] resp_filename if a specific file needs to be created us this name, if NULL resp_fd will be a temporary file
 * @param[in,out] resp_fd stream for the output resp.
 * @retval EVALRESP_OK on success
 * @pre xml_fd must not be NULL 
 * @post *resp_fd will point to a FILE stream that must be closed
 * @brief do conversion of xml -> resp files but stored as a File
 */
int evalresp_xml_stream_to_resp_file(evalresp_logger *log, int xml_flag, FILE *xml_fd, const char * resp_filename, FILE **resp_fd);

/**
 * @param[in] log logging structure where you want information to be sent
 * @param[in] xml_fd stream containing xml information
 * @parma[in] resp_filename if a specific file needs to be created us this name, if NULL resp_fd will be a temporary file
 * @param[in,out] resp_fd stream for the output resp.
 * @retval EVALRESP_OK on success
 * @pre xml_fd must not be NULL 
 * @post *resp_fd will point to a FILE stream that must be closed
 * @brief automatically do conversion of xml -> resp files if needed
 */
int evalresp_xml_stream_to_resp_stream_auto(evalresp_logger *log, FILE *xml_fd, const char * resp_filename, FILE **resp_fd);
#endif /* __EVALRESP_X2R_WRAPPERS_H__ */
