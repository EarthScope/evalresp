
#ifndef X2R_DOM_TO_CHANNELS_H
#define X2R_DOM_TO_CHANNELS_H

#include <evalresp/public_api.h>
#include <evalresp/stationxml2resp/xml_to_dom.h>
#include <evalresp_log/log.h>

/**
 * @private
 * @ingroup evalresp_private_x2r_xml
 * @brief Build evalresp channels directly from the in-memory model of a
 *        station.xml document, without an intermediate RESP text.
 * @details The channels are those that parsing the equivalent RESP text would
 *          produce, but with values at full precision.
 * @param[in] log logging structure
 * @param[in] root the parsed document
 * @param[in] options evalresp options (used to interpret units)
 * @param[out] channels the new channels (free with evalresp_free_channels)
 * @retval EVALRESP_OK on success
 */
int x2r_to_channels(evalresp_logger *log, const x2r_fdsn_station_xml *root,
        evalresp_options const *options, evalresp_channels **channels);

#endif
