
#ifndef EVALRESP_PUBLIC_H
#define EVALRESP_PUBLIC_H

#include "evalresp/evalresp_public_seed.h"
#include "evalresp_log/log.h"

/* IGD 10/16/04 This is for Windows which does not use Makefile.am */
#ifdef VERSION
#define REVNUM VERSION
#else
#define REVNUM "4.0.6"
#endif

// once this is defined (v4.0.6 onwards), some functions take an additional
// xml flag argument
#define EVRESP_XML

/**
 * @private
 * @ingroup evalresp_private
 * @brief Evaluate responses for user requested station/channel/network tuple
 *        at the frequencies requested by the user.
 * @param[in] stalst Station list.
 * @param[in] chalst Channel list.
 * @param[in] net_code Network code.
 * @param[in] locidlst Localtion ID list.
 * @param[in] date_time Date + time.
 * @param[in] units Units.
 * @param[in] file File name.
 * @param[in] freqs Frequency vector.
 * @param[in] nfreqs Number of frequencies in the vector.
 * @param[in] rtype Use complex value or Amplited/phase in the output.
 * @param[in] verbose If used, then verbose.
 * @param[in] start_stage Start stage.
 * @param[in] stop_stage End stage.
 * @param[in] stdio_flag Print output to stdio if used.
 * @param[in] useTotalSensitivityFlag Use or not total sensitivity.
 * @param[in] x_for_b62 Frequency value for Polynomial.
 * @param[in] xml_flag Use XML or not.
 * @param[in] log Logging structure.
 * @remark Calls evresp_itp() but with listinterp_tension set to 0.
 * @returns Responses.
 */
struct response *evresp (char *stalst, char *chalst, char *net_code,
                         char *locidlst, char *date_time, char *units,
                         char *file, double *freqs, int nfreqs, char *rtype,
                         char *verbose, int start_stage, int stop_stage,
                         int stdio_flag, int useTotalSensitivityFlag,
                         double x_for_b62, int xml_flag, evalresp_log_t *log);

/**
 * @private
 * @ingroup evalresp_private
 * @brief Evaluate responses for user requested station/channel/network tuple
 *        at the frequencies requested by the user.
 * @param[in] stalst Station list.
 * @param[in] chalst Channel list.
 * @param[in] net_code Network code.
 * @param[in] locidlst Localtion ID list.
 * @param[in] date_time Date + time.
 * @param[in] units Units.
 * @param[in] file File name.
 * @param[in] freqs Frequency vector.
 * @param[in] nfreqs Number of frequencies in the vector.
 * @param[in] rtype Use complex value or Amplited/phase in the output.
 * @param[in] verbose If used, then verbose.
 * @param[in] start_stage Start stage.
 * @param[in] stop_stage End stage.
 * @param[in] stdio_flag Print output to stdio if used.
 * @param[in] listinterp_out_flag Interpolate output of list (B55).
 * @param[in] listinterp_in_flag Interpolate input of list (B55).
 * @param[in] listinterp_tension This parameter is obsolote and should be removed.
 * @param[in] useTotalSensitivityFlag Use or not total sensitivity.
 * @param[in] x_for_b62  Frequency value for Polynomial.
 * @param[in] xml_flag Use XML or not.
 * @param[in] log Logging structure.
 * @returns Responses.
 */
struct response *evresp_itp (char *stalst, char *chalst, char *net_code,
                             char *locidlst, char *date_time, char *units,
                             char *file, double *freqs, int nfreqs,
                             char *rtype, char *verbose, int start_stage,
                             int stop_stage, int stdio_flag,
                             int listinterp_out_flag, int listinterp_in_flag,
                             double listinterp_tension,
                             int useTotalSensitivityFlag, double x_for_b62,
                             int xml_flag, evalresp_log_t *log);

/**
 * @private
 * @ingroup evalresp_private
 * @brief Evaluate responses for user requested station/channel/network tuple
 *        at the frequencies requested by the user. FORTRAN callable interface to the evresp.
 *
 *        This routine was updated in release 4.0.0 to support Fortran
 *        95.  Previous versions were clearly broken (parameters had
 *        been added with no respect to the implicit lengths from
 *        Fortran character arrays), so backwards compatibility is not
 *        provided.
 *        Given the cleaner interface supported by Fortran 95, this
 *        routine can also be called from C.
 *        Whereas the other function returns a linked list of responses
 *        one for each response that matched the user's request), this
 *        routine returns the response for one (1)
 *        station-channel-network for one (1) effective time.  If more
 *        than one match is found for a given
 *        station-channel-network-time, an error condition is raised
 *        (and a value of -1 is returned to the calling routine to
 *        indicate failure).  Likewise, a value of 1 is returned if no
 *        match is found for the given station-channel-network-time.  If
 *        a unique match is found, a value of 0 is returned to the
 *        calling routine
 *
 * @param[in] sta Station.
 * @param[in] cha Channel.
 * @param[in] net Network.
 * @param[in] locid Localtion ID.
 * @param[in] datime data and time.
 * @param[in] units Units.
 * @param[in] file File name.
 * @param[in] freqs Frequency vector.
 * @param[in] nfreqs Number of frequencies in the vector.
 * @param[out] resp Double vector containing complex response (even elements are real).
 * @param[in] rtype Use complex value or Amplited/phase in the output.
 * @param[in] verbose If used, then verbose.
 * @param[in] start_stage Start stage..
 * @param[in] stop_stage End stage..
 * @param[in] stdio_flag Print output to stdio if used.
 * @param[in] useTotalSensitivityFlag Use or not total sensitivity.
 * @param[in] x_for_b62 Frequency value for Polynomial.
 * @param[in] xml_flag Use XML or not.
 * @remark Fortran interface.
 * @note this will log to default of the library, currently stderr
 */
int evresp_1 (char *sta, char *cha, char *net, char *locid, char *datime,
              char *units, char *file, double *freqs, int nfreqs, double *resp,
              char *rtype, char *verbose, int start_stage, int stop_stage,
              int stdio_flag, int useTotalSensitivityFlag, double x_for_b62,
              int xml_flag);

#endif
