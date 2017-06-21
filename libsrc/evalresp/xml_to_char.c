#include <stdlib.h>

#include <mxml/mxml.h>
#include <evalresp_log/log.h>

#include "evalresp/public_api.h"
#include "evalresp/stationxml2resp.h"
#include "evalresp/stationxml2resp/ws.h"
#include "evalresp/stationxml2resp/xml.h"

int evalresp_convert_xml_to_char (evalresp_log_t *log, char *xml_in, char **resp_out);
int evalresp_load_mxml_service (evalresp_log_t *log, char *xml_in, x2r_fdsn_station_xml **root);
int evalresp_save_mxml_service_to_char (evalresp_log_t *log, x2r_fdsn_station_xml *root, char **resp_out);
extern int parse_fdsn_station_xml(evalresp_log_t *log, mxml_node_t *doc, x2r_fdsn_station_xml **root);

int
evalresp_xml_to_char (evalresp_log_t *log, int xml_flag, char *xml_in, char **resp_out)
{
  if (!xml_flag)
  {
    return EVALRESP_OK;
  }
  return evalresp_convert_xml_to_char(log, xml_in, resp_out);
}

int
evalresp_convert_xml_to_char (evalresp_log_t *log, char *xml_in, char **resp_out)
{
  int status;
  x2r_fdsn_station_xml *root = NULL;

  if (EVALRESP_OK == (status = evalresp_load_mxml_service (log, xml_in, &root)))
  {
    status = evalresp_save_mxml_service_to_char (log, root, resp_out);
  }
  return status;
}

int
evalresp_load_mxml_service (evalresp_log_t *log, char *xml_in, x2r_fdsn_station_xml **root)
{
  int status;
  mxml_node_t *doc = NULL;

  if (!xml_in)
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Input XML Sting is NULL");
    return EVALRESP_IO;
  }

  if (!(doc = mxmlLoadString (NULL, xml_in, MXML_OPAQUE_CALLBACK)))
  {
    evalresp_log (log, EV_ERROR, EV_ERROR, "Could not parse XML input");
    status = EVALRESP_XML_ERR;
  }
  else
  {
    status = parse_fdsn_station_xml (log, doc, root);
  }

  mxmlDelete (doc);
  return status;
}

int
evalresp_save_mxml_service_to_char (evalresp_log_t *log, x2r_fdsn_station_xml *root, char **resp_out)
{
  FILE *tmp_fd = NULL;
  int length = 0, status;
  char *buffer = NULL;
  if (*resp_out)
  {
    evalresp_log (log, EV_WARN, EV_WARN, "Output Response string is not NULL and will be lost");
  }
  /* create temporary file */
  if (NULL == (tmp_fd = tmpfile()))
  {
      evalresp_log(log, EV_ERROR, EV_ERROR, "Cannot create temporary file for xml to resp chars ");
      return EVALRESP_IO;
  }
  status = x2r_resp_util_write(log, tmp_fd, root);
  if (EVALRESP_OK == status)
  {
      fflush(tmp_fd);
      if (fseek(tmp_fd, 0, SEEK_END))
      {
          evalresp_log(log, EV_ERROR, EV_ERROR, "Problem with temporary file");
          status = EVALRESP_IO;
      }
      else
      {
          if (0 > (length = ftell(tmp_fd)))
          {
            evalresp_log(log, EV_ERROR, EV_ERROR, "Problem with getting length of temporary file");
            status = EVALRESP_IO;
          }
          rewind(tmp_fd);
      }
      
  }
  if (0 < length)
  {
      length++; /* account for null termination from this point on */
      if ((buffer = (char *) calloc(length, sizeof(char))))
      {
        fread((void *)buffer, sizeof(char), length, tmp_fd);
      } else {
          evalresp_log(log, EV_ERROR, EV_ERROR, "Could not allocate buffer fore xml to resp output");
          status = EVALRESP_MEM;
      }
  }
  *resp_out = buffer;
  /* close and delete temporary file */
  if (tmp_fd)
  {
      fclose(tmp_fd);
  }
  
  return status;
}
#if 0
/* Read an x2r_fdsn_station_xml value from the current node. */
static int parse_fdsn_station_xml(evalresp_log_t *log, mxml_node_t *doc, x2r_fdsn_station_xml **root) {

    int status = EVALRESP_OK, i;
    mxml_node_t *fdsn = NULL;
    nodelist *networks = NULL;

    //evalresp_log(log, EV_DEBUG, 0, "Parsing root");
    /*XXX x2r_debug(log, "Parsing root"); */

    if (!(*root = calloc(1, sizeof(**root)))) {
        evalresp_log(log, EV_ERROR, 0, "Cannot alloc fdsn_station_xml");
        status = EVALRESP_MEM;
        /*XXX status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc fdsn_station_xml"); */
    } else {
        if (!(status = find_child(log, &fdsn, NULL, doc, "FDSNStationXML"))) {
            if (!(status = find_children(log, &networks, fdsn, "Network"))) {
                (*root)->n_networks = networks->n;
                if (!((*root)->network = calloc((*root)->n_networks, sizeof(*(*root)->network)))) {
                    evalresp_log(log, EV_ERROR, 0, "Cannot alloc networks");
                    status = X2R_ERR_MEMORY;
                    /*XXX status = x2r_error(log, X2R_ERR_MEMORY, "Cannot alloc networks"); */
                } else {
                    for (i = 0; !status && i < (*root)->n_networks; ++i) {
                        status = parse_network(log, networks->node[i], &(*root)->network[i]);
                    }
                }
            }
        }
    }

    free_nodelist(&networks);
    return status;
}
#endif
