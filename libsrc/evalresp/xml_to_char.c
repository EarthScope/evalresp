#include <stdlib.h>

#include <evalresp_log/log.h>

#include "evalresp/stationxml2resp.h"
#include "evalresp/public_api.h"

int evalresp_convert_xml_to_char(evalresp_log_t *log, char *xml_in, char **resp_out);
int evalresp_load_mxml_service(evalresp_log_t *log, char *xml_in, x2r_fdsn_station_xml **root);
int evalresp_save_mxml_service_to_char(evalresp_log_t *log, x2r_fdsn_station_xml *root, char **resp_out);

int evalresp_xml_to_char (evalresp_log_t *log, int xml_flag, char *xml_in, char **resp_out)
{
    if (!xml_flag)
    {
        return EVALRESP_OK;
    }
    return evalresp_convert_xml_to_char;
}

int evalresp_convert_xml_to_char(evalresp_log_t *log, char *xml_in, char **resp_out)
{
    int status;
    x2r_fdsn_station_xml *root = NULL;

    if (EVALRESP_OK == (status = evalresp_load_mxml_service (log, xml_in, &root)))
    {
        status = evalresp_save_mxml_service_to_char(log, root, resp_out);
    }
    return status;
}

int evalresp_load_mxml_service(evalresp_log_t *log, char *xml_in, x2r_fdsn_station_xml **root)
{
    int status;
    mxml_node_t *doc = NULL;
    
    if (!xml_in)
    {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Input XML Sting is NULL");
        return EVALRESP_IO;
    }

    if (!(doc = mxmlLoadString(NULL, xml_in, MXML_OPAQUE_CALLBACK)))
    {
        evalresp_log(log, EV_ERROR, EV_ERROR, "Could not parse XML input");
        status = EVALRESP_XML_ERR;
    }
    else
    {
        status = parse_fdsn_station_xml(log, doc, root);
    }
    
    mxmlDelete(doc);
    return status;
}

int evalresp_save_mxml_service_to_char(evalresp_log_t *log, x2r_fdsn_station_xml *root, char **resp_out)
{
    int total_chans = 0;
    if (*resp_out)
    {
        evalresp_log(log, EV_WARN, EV_WARN, "Output Response string is not NULL and will be lost");
    }
    for (i = 0; !status && i < root->n_networks; i++)
    {
        network = root->network + i;
        for (j = 0; !status && j < network->n_stations; j++)
        {
            station = network->station + j;
            total_chans += station->n_channels;
        }
    }
    buff_size = 289 * total_chans + 1;
    *resp_out = (char *) calloc(buff_size, sizeof(char));

    for (i = 0; !status && i < root->n_networks; i++)
    {
        network = root->network + i;
        for (j = 0; !status && j < network->n_stations; j++)
        {
            station = network->station + j;
            for (k =0; !status && k < station->n_channels; k++)
            {
                channel = staion->channel + k;
                snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), "#\n");
                snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), 
        "###################################################################################\n");
                snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), "#\n");
                snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), 
        "B050F03     Station:     %s\n", stn);
                snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), 
        "B050F16     Network:     %s\n", net);
                snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), 
        "B052F03     Location:    %s\n", (!(strcmp(channel->location_code, "  ") && strcmp(channel->location_code, ""))) ?
                                "??" : channel->location_code);
                snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), 
        "B052F04     Channel:     %s\n", channel->code);
                if (!(status = format_date_yjhms(log, channel->start_date, &start)))
                {
                    snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), 
                             "B052F22     Start date:  %s\n", start);
                }
                if (!status)
                {
                    if (!(status = format_date_yjhms(log, channel->end_date, &end)))
                    {
                        snprintf(*resp_out + strnlen(resp_out,buff_size), buff_size - strnlen(resp_out,buff_size), 
                                 "B052F23     End date:    %s\n", end);
                    }
                }
                if (!status)
                {
                    /*TODO status = print_response(log, out, net, stn, channel, &channel->response);*/
                }
            }
        }
    }
    return status;
}
