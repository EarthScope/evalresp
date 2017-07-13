
#ifndef EVALRESP_PUBLIC_COMPAT_H
#define EVALRESP_PUBLIC_COMPAT_H

#include "evalresp/public_channels.h"
#include "evalresp_log/log.h"

// TODO - this should become evresp.h eventually

// once this is defined (v4.0.6 onwards), some functions take an additional
// xml flag argument
#define EVRESP_XML

/**
 * @public
 * @ingroup evalresp_public_compat
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
evalresp_response *evresp (char *stalst, char *chalst, char *net_code,
                           char *locidlst, char *date_time, char *units,
                           char *file, double *freqs, int nfreqs, char *rtype,
                           char *verbose, int start_stage, int stop_stage,
                           int stdio_flag, int useTotalSensitivityFlag,
                           double x_for_b62, int xml_flag, evalresp_log_t *log);

/**
 * @public
 * @ingroup evalresp_public_compat
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

/**
 * @public
 * @ingroup evalresp_public_compat
 * @brief Evaluate responses for user requested station/channel/network tuple
 *        at the frequencies requested by the user. This function provides an interface
 *        to both evresp() and evresp_1()

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
evalresp_response *evresp_itp (char *stalst, char *chalst, char *net_code,
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
 * @brief Small function to set and return a static flag to use or not use
 *        the estimated delay in response computation.
 * @details The reason we want to use this global variable is because we don't
u *          want to change the number of arguments in evresp() function which
 *          is used in users programs.
 * @param[in] flag NEGATIVE means that we want to query the value of the flag
 *                 TRUE or FALSE means that we want to set corresponding
 *                 values.
 * @author 03/01/05: IGD.
 */
int use_estimated_delay (int flag);

/**
 * @private
 * @ingroup evalresp_private_calc
 * @brief Calculate response.
 * @param[in] chan Channel structure.
 * @param[in] freq Frequency array.
 * @param[in] nfreqs Number if numbers in @p freq.
 * @param[in] output Output.
 * @param[in] out_units Units of output.
 * @param[in] start_stage Start stage.
 * @param[in] stop_stage Stop stage.
 * @param[in] useTotalSensitivityFlag Use reported sensitivity to compute
 *                                    response.
 * @param[in] x_for_b62 Frequency for polynomial response (b62).
 * @param[in] log Logging structure.
 */
void calc_resp (evalresp_channel *chan, double *freq, int nfreqs,
                evalresp_complex *output, const char *out_units, int start_stage,
                int stop_stage, int useTotalSensitivityFlag, double x_for_b62,
                evalresp_log_t *log);
#endif
