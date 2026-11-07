/*
 * Runs the evalresp and xml2resp programs on fixture files and compares what
 * they write with the files under expected/cli/<expected>.
 *
 * Each case runs in an empty work directory holding a copy of its input
 * file.  The files left there afterwards (other than the input) must be
 * exactly the files in the expected directory, unless ignore_extra is set.
 */

#include <stdio.h>
#include <string.h>

#include "harness/harness.h"

typedef struct
{
  const char *name;
  const char *expected; /* directory under expected/cli; NULL if no output files are expected */
  const char *program;
  const char *input; /* fixture copied into the work directory */
  const char *argv[32];
  int stdin_from_input;
  const char *stdout_to; /* file receiving stdout, or NULL to inherit */
  const char *stderr_to;
  int stdout_cols;  /* numeric columns in the stdout file; 0 for text */
  int rc;           /* expected exit status */
  const char *only; /* comma-separated subset of the expected files to compare */
  int ignore_extra; /* allow files that are not in the expected directory */
  double tol;       /* 0 selects the default */
  int absolute;
} cli_case;

#define CGV_FILE "RESP.Z.CGV..HYZ"
#define ANMO_FILE "RESP.IU.ANMO.00.BHZ"
#define ATTU_FILE "RESP.IM.ATTU..BHE"
#define SAVY_FILE "RESP.CT.SAVY..BHE"

#define CGV "CGV", "HYZ", "2010", "1", "0.001", "10", "100", "-f", CGV_FILE
#define ANMO "ANMO", "BHZ", "2010", "1", "0.001", "10", "100", "-f", ANMO_FILE
#define ATTU "ATTU", "BHE", "1998", "57", "0.001", "10", "100", "-f", ATTU_FILE
#define SAVY                                                                                                         \
  "SAVY", "BHE", "1995", "265", "1.0E-5", "20.0", "200", "-f", SAVY_FILE, "-u", "def", "-r", "ap", "-n", "CT", "-l", \
      "  ", "-t", "04:25:27", "-s", "log", "-v"

static const cli_case cases[] = {
    /* basic calls */
    {.name = "simple_direct", .expected = "simple/direct", .program = "evalresp", .input = CGV_FILE, .argv = {CGV}},
    {.name = "simple_bad_date",
     .program = "evalresp",
     .input = CGV_FILE,
     .argv = {"CGV", "HYZ", "2009", "365", "0.001", "10", "100", "-f", CGV_FILE}},
    {.name = "simple_bad_site",
     .program = "evalresp",
     .input = CGV_FILE,
     .argv = {"XXX", "HYZ", "2009", "365", "0.001", "10", "100", "-f", CGV_FILE}},
    {.name = "simple_bad_channel",
     .program = "evalresp",
     .input = CGV_FILE,
     .argv = {"CGV", "XXX", "2009", "365", "0.001", "10", "100", "-f", CGV_FILE}},
    {.name = "simple_wildcards",
     .expected = "simple/direct",
     .program = "evalresp",
     .input = CGV_FILE,
     .argv = {"*", "*", "2010", "1", "0.001", "10", "100", "-f", CGV_FILE}},
    {.name = "simple_direct_2",
     .expected = "simple/direct_2",
     .program = "evalresp",
     .input = ATTU_FILE,
     .argv = {ATTU}},
    {.name = "simple_filter",
     .expected = "simple/filter",
     .program = "evalresp",
     .input = "RESP.H2.H2O.00.HHZ",
     .argv = {"H2O", "HHZ", "2001", "235", "1.0E-5", "160.0", "200", "-f", "RESP.H2.H2O.00.HHZ", "-u", "def", "-r",
              "fap", "-n", "H2", "-l", "00", "-t", "08:16:02", "-s", "log", "-v"},
     .stdout_to = "stdout",
     .stderr_to = "stderr",
     .ignore_extra = 1},
    {.name = "simple_filter_2", .expected = "simple/filter_2", .program = "evalresp", .input = SAVY_FILE, .argv = {SAVY}},
    {.name = "simple_open",
     .expected = "simple/open",
     .program = "evalresp",
     .input = "TITA1.HGZ",
     .argv = {"TITA1", "HGZ", "2019", "1", "0.001", "10", "100", "-f", "TITA1.HGZ", "-x"}},

    /* options */
    {.name = "args_units_dis", .expected = "args/units_dis", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-u", "dis"}},
    {.name = "args_units_vel", .expected = "args/units_vel", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-u", "vel"}},
    {.name = "args_units_acc", .expected = "args/units_acc", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-u", "acc"}},
    {.name = "args_units_def", .expected = "args/units_def", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-u", "def"}},
    {.name = "args_time_of_day", .expected = "args/time_of_day", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-t", "06:54:32"}},
    {.name = "args_log_space", .expected = "args/log_space", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-s", "log"}},
    {.name = "args_linear_space", .expected = "args/linear_space", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-s", "lin"}},
    {.name = "args_amp_phase", .expected = "args/amp_phase", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-r", "ap"}},
    {.name = "args_freq_amp_phase", .expected = "args/freq_amp_phase", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-r", "fap"}},
    {.name = "args_complex", .expected = "args/complex", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-r", "cs"}},
    {.name = "args_net_id", .expected = "args/net_id", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-n", "Z"}},
    {.name = "args_bad_net_id", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-n", "XXX"}},
    {.name = "args_location_id", .expected = "args/location_id", .program = "evalresp", .input = ANMO_FILE, .argv = {ANMO, "-l", "00"}},
    {.name = "args_location_id_wild_1", .expected = "args/location_id_wild_1", .program = "evalresp", .input = ANMO_FILE, .argv = {ANMO, "-l", "??"}},
    {.name = "args_location_id_wild_2", .expected = "args/location_id_wild_2", .program = "evalresp", .input = ANMO_FILE, .argv = {ANMO, "-l", "*"}},
    {.name = "args_bad_location_id", .program = "evalresp", .input = ANMO_FILE, .argv = {ANMO, "-l", "01"}},
    {.name = "args_stage_3", .expected = "args/stage_3", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-stage", "3"}},
    {.name = "args_stage_3_5", .expected = "args/stage_3_5", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-stage", "3", "5"}},
    {.name = "args_stdio",
     .expected = "args/stdio",
     .program = "evalresp",
     .input = CGV_FILE,
     .argv = {"CGV", "HYZ", "2010", "1", "0.001", "10", "100", "-stdio"},
     .stdin_from_input = 1,
     .stdout_to = "stdout",
     .stderr_to = "stderr",
     .stdout_cols = 3},
    {.name = "args_delay", .expected = "args/delay", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-use-estimated-delay"}},
    {.name = "args_interp_out", .expected = "args/interp_out", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-il"}},
    {.name = "args_interp_in", .expected = "args/interp_in", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-ii"}},
    {.name = "args_tension", .expected = "args/tension", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-ii", "-il", "-it", "100"}},
    {.name = "args_unwrap", .expected = "args/unwrap", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-unwrap"}},
    /* The expected files are reference output from another implementation:
       the amplitudes are not comparable, and the phase is compared only
       to within 360 degrees. */
    {.name = "args_unwrap_iris_no_flag",
     .expected = "args/unwrap-iris-no-flag",
     .program = "evalresp",
     .input = SAVY_FILE,
     .argv = {SAVY},
     .stdout_to = "stdout",
     .stderr_to = "stderr",
     .only = "PHASE.CT.SAVY..BHE",
     .ignore_extra = 1,
     .tol = 360,
     .absolute = 1},
    {.name = "args_unwrap_iris_flag",
     .expected = "args/unwrap-iris-flag",
     .program = "evalresp",
     .input = SAVY_FILE,
     .argv = {SAVY, "-unwrap"},
     .stdout_to = "stdout",
     .stderr_to = "stderr",
     .only = "PHASE.CT.SAVY..BHE",
     .ignore_extra = 1,
     .tol = 360,
     .absolute = 1},
    {.name = "args_total_sens", .expected = "args/total_sens", .program = "evalresp", .input = CGV_FILE, .argv = {CGV, "-ts"}},
    {.name = "args_verbose",
     .expected = "args/verbose",
     .program = "evalresp",
     .input = CGV_FILE,
     .argv = {CGV, "-v"},
     .stdout_to = "stdout",
     .stderr_to = "stderr"},
    {.name = "args_xml",
     .expected = "args/xml",
     .program = "evalresp",
     .input = "station-1.xml",
     .argv = {"ANMO", "BHZ", "2015", "1", "0.001", "10", "100", "-f", "station-1.xml", "-x", "-l", "10"}},
    {.name = "args_b55",
     .expected = "args/b55",
     .program = "evalresp",
     .input = ATTU_FILE,
     .argv = {ATTU, "-il"}},

    /* xml2resp */
    {.name = "xml2resp_file",
     .expected = "xml2resp/file",
     .program = "xml2resp",
     .input = "station-1.xml",
     .argv = {"-o", "RESP", "station-1.xml"}},
    {.name = "xml2resp_pipe",
     .expected = "xml2resp/pipe",
     .program = "xml2resp",
     .input = "station-1.xml",
     .stdin_from_input = 1,
     .stdout_to = "RESP"},
    {.name = "xml2resp_open",
     .expected = "xml2resp/open",
     .program = "xml2resp",
     .input = "TITA1.HGZ",
     .stdin_from_input = 1,
     .stdout_to = "RESP"},
};

#define N_CASES (sizeof (cases) / sizeof (cases[0]))

static er_test table[N_CASES + 1];

static int
in_list (const char *list, const char *name)
{
  size_t n = strlen (name);
  const char *p = list;
  while (p && *p)
  {
    const char *end = strchr (p, ',');
    size_t len = end ? (size_t)(end - p) : strlen (p);
    if (len == n && !strncmp (p, name, n))
      return 1;
    p = end ? end + 1 : NULL;
  }
  return 0;
}

static int
in_names (char **names, int n, const char *name)
{
  int i;
  for (i = 0; i < n; ++i)
    if (!strcmp (names[i], name))
      return 1;
  return 0;
}

static er_cmp
options_for (const cli_case *c, const char *name)
{
  er_cmp o;
  memset (&o, 0, sizeof (o));
  if (!strncmp (name, "AMP.", 4) || !strncmp (name, "PHASE.", 6))
    o.ncols = 2;
  else if (!strncmp (name, "FAP.", 4) || !strncmp (name, "SPECTRA.", 8))
    o.ncols = 3;
  else if (!strcmp (name, "stdout"))
    o.ncols = c->stdout_cols;
  else if (!strcmp (name, "stderr"))
    o.log_text = 1;
  o.tol = c->tol;
  o.absolute = c->absolute;
  return o;
}

static void
check_outputs (const cli_case *c)
{
  char **produced = NULL, **expected = NULL;
  char dir[ER_PATH_MAX], result[ER_PATH_MAX], target[ER_PATH_MAX];
  int n_produced, n_expected = 0, i;

  ER_CHECK_MSG ((n_produced = er_list_files (".", &produced)) >= 0, "cannot list the work directory");

  if (!c->expected)
  {
    for (i = 0; i < n_produced; ++i)
      ER_CHECK_MSG (!strcmp (produced[i], c->input), "unexpected output file %s", produced[i]);
    er_free_names (produced, n_produced);
    return;
  }

  snprintf (dir, sizeof (dir), "cli/%s", c->expected);
  snprintf (target, sizeof (target), "%s", er_expected_path (dir));
  ER_CHECK_MSG ((n_expected = er_list_files (target, &expected)) > 0, "no expected files in %s", target);

  if (!c->ignore_extra)
  {
    for (i = 0; i < n_produced; ++i)
      ER_CHECK_MSG (!strcmp (produced[i], c->input) || in_names (expected, n_expected, produced[i]),
                    "unexpected output file %s", produced[i]);
  }

  for (i = 0; i < n_expected; ++i)
  {
    er_cmp o;
    if (c->only && !in_list (c->only, expected[i]))
      continue;
    ER_CHECK_MSG (in_names (produced, n_produced, expected[i]), "missing output file %s", expected[i]);
    o = options_for (c, expected[i]);
    snprintf (result, sizeof (result), "%s", expected[i]);
    snprintf (dir, sizeof (dir), "%s/%s", target, expected[i]);
    er_assert_files_match (result, dir, &o);
  }
  er_free_names (produced, n_produced);
  er_free_names (expected, n_expected);
}

static void
run_case (void)
{
  const cli_case *c = er_test_ctx ();
  const char *argv[40];
  int n = 0, rc;

  ER_CHECK_MSG (!er_copy_file (er_data_path (c->input), c->input), "cannot copy %s", c->input);

  argv[n++] = er_bin_path (c->program);
  while (c->argv[n - 1])
  {
    argv[n] = c->argv[n - 1];
    ++n;
  }
  argv[n] = NULL;

  rc = er_run (argv, c->stdin_from_input ? c->input : NULL, c->stdout_to, c->stderr_to);
  ER_CHECK_MSG (rc == c->rc, "%s exited with status %d, expected %d", c->program, rc, c->rc);
  check_outputs (c);
}

const er_test *
cli_tests (void)
{
  size_t i;
  for (i = 0; i < N_CASES; ++i)
  {
    table[i].name = cases[i].name;
    table[i].fn = run_case;
    table[i].ctx = &cases[i];
  }
  return table;
}
