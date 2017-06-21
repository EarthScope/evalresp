#include <stdlib.h>

#include <mxml/mxml.h>
#include <evalresp_log/log.h>

#include "evalresp/public_api.h"
#include "evalresp/input.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/ws.h"
#include "evalresp/stationxml2resp/xml.h"

/**
 * @private
 * @param[in] log logging structure where you want information to be sent
 * @param[in] root the root node of the xml converted to c stuructures
 * @param[in,out] resp_out a pointer where to allocate and store the translated response file as char *
 * @retval EVALRESP_OK on success
 * @post *resp_out will be allocated using calloc that must be freed
 * @brief convert mxml generated structure to a response file storred as char *
 */
static int
save_mxml_service_to_char (evalresp_log_t *log, x2r_fdsn_station_xml *root, char **resp_out)
{
  FILE *tmp_fd = NULL;
  //int length = 0;
  int status;
  char *buffer = NULL;
  if (*resp_out)
  {
    evalresp_log (log, EV_WARN, EV_WARN, "Output Response string is not NULL and will be lost");
  }
  /* create temporary file will be deleted on fclose*/
  if (NULL == (tmp_fd = tmpfile()))
  {
      evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot create temporary file for xml to resp chars ");
      return EVALRESP_IO;
  }
  /* write mxml structure as response file in the tmeporary ffile */
  status = x2r_resp_util_write(log, tmp_fd, root);
  if (EVALRESP_OK == status)
  {
    /* guarentee everything was written */
    fflush(tmp_fd);
    /* want to read from the begining of the file */
    rewind(tmp_fd);
    /* pull file into string */
    status = file_to_char (log, tmp_fd, &buffer);
  }

  /* regardless of success set the resp_out to the buffer */
  *resp_out = buffer;
  /* close and delete temporary file */
  if (tmp_fd)
  {
      fclose(tmp_fd);
  }
  
  return status;
}

/**
 * @private
 * @param[in] log logging structure where you want information to be sent
 * @param[in] xml_in char * containing xml to be translated
 * @param[out] root the root node of the xml converted to c stuructures
 * @retval EVALRESP_OK on success
 * @sa x2r_fdsn_station_xml
 * @pre xml_in must not be NULL
 * @post root must be freed
 * @brief load the xml string into a mxml generated structure
 */
static int
load_mxml_service (evalresp_log_t *log, char *xml_in, x2r_fdsn_station_xml **root)
{
  int status;
  mxml_node_t *doc = NULL;

  /* check that xml_in is set */
  if (!xml_in)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Input XML String is NULL");
    return EVALRESP_IO;
  }

  /* load the string into mxml */
  if (!(doc = mxmlLoadString (NULL, xml_in, MXML_OPAQUE_CALLBACK)))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Could not parse XML input");
    status = EVALRESP_XML_ERR;
  }
  else
  {
    /* parse the mxml into data structure */
    status = x2r_parse_fdsn_station_xml (log, doc, root);
  }

  /* clean up mxml */
  mxmlDelete (doc);
  return status;
}

/**
 * @private
 * @param[in] log logging structure where you want information to be sent
 * @param[in] xml_in char * containing xml to be translated
 * @param[in,out] resp_out a pointer where to allocate and store the translated response file as char *
 * @retval EVALRESP_OK on success
 * @pre xml_in must not be NULL and *resp_out must be NULL
 * @post *resp_out will be allocated using calloc that must be freed
 * @brief house keeping function to initiate xml -> resp files but stored as char *
 */
static int
convert_xml_to_char (evalresp_log_t *log, char *xml_in, char **resp_out)
{
  int status;
  x2r_fdsn_station_xml *root = NULL;

  /* load xml string and parse */
  if (EVALRESP_OK == (status = load_mxml_service (log, xml_in, &root)))
  {
    /* convert xml data structure to resp string */
    status = save_mxml_service_to_char (log, root, resp_out);
  }
  /* clean up */
  status = x2r_free_fdsn_station_xml(root, status);
  return status;
}

int
evalresp_xml_to_char (evalresp_log_t *log, int xml_flag, char *xml_in, char **resp_out)
{
  /* check to make sure we want this */
  if (!xml_flag)
  {
    return EVALRESP_OK;
  }
  /* call underlying functions */
  return convert_xml_to_char(log, xml_in, resp_out);
}

