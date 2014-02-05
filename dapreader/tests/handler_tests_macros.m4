
# 
# These macros are used for both the netcdf3 and netcdf4 tests.

AT_INIT([bes.conf besstandalone getdap])
# AT_COPYRIGHT([])

AT_TESTED([besstandalone])

AT_ARG_OPTION_ARG([generate g],
    [  -g arg, --generate=arg   Build the baseline file for test 'arg'],
    [if besstandalone -c bes.conf -i $at_arg_generate -f $at_arg_generate.baseline; then
         echo "Built baseline for $at_arg_generate"
     else
         echo "Could not generate baseline for $at_arg_generate"
     fi     
     exit],[])

dnl echo "besstandalone -c bes.conf -i $at_arg_generate_dap  | getdap4 -D -M - > $at_arg_generate_dap.baseline"

AT_ARG_OPTION_ARG([generate-dap a],
    [  -a arg, --generate-dap=arg   Build the baseline file for test 'arg'],
    [if besstandalone -c bes.conf -i $at_arg_generate_dap  | getdap4 -D -M - > $at_arg_generate_dap.baseline; then
         echo "Built baseline for $at_arg_generate_dap"
     else
         echo "Could not generate baseline for $at_arg_generate_dap"
     fi     
     exit],[])

# Usage: _AT_TEST_*(<bescmd source>, <baseline file>)

m4_define([AT_BESCMD_RESPONSE_TEST],
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([bescmd])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $abs_srcdir/$1 || true], [], [stdout], [stderr])
AT_CHECK([diff -b -B $abs_srcdir/$1.baseline stdout || diff -b -B $abs_srcdir/$1.baseline stderr], [], [ignore],[],[])
AT_CLEANUP])

m4_define([AT_BESCMD_PATTERN_RESPONSE_TEST],
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([bescmd])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $abs_srcdir/$1 || true], [], [stdout], [stderr])
AT_CHECK([grep -f $abs_srcdir/$1.baseline stdout], [], [ignore],[],[])
AT_CLEANUP])

# DAP2 data responses

m4_define([AT_BESCMD_BINARYDATA_RESPONSE_TEST],
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([bescmd])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $abs_srcdir/$1 | getdap -M - || true], [], [stdout], [stderr])
AT_CHECK([diff -b -B $abs_srcdir/$1.baseline stdout || diff -b -B $abs_srcdir/$1.baseline stderr], [], [ignore],[],[])
AT_CLEANUP])

m4_define([AT_BESCMD_PATTERN_DATA_RESPONSE_TEST],
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([bescmd])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $abs_srcdir/$1 | getdap -M - || true], [], [stdout], [stderr])
AT_CHECK([grep -f $abs_srcdir/$1.baseline stdout], [], [ignore],[],[])
AT_CLEANUP])

# DAP4 Data responses

m4_define([AT_BESCMD_DAP_RESPONSE_TEST],
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([bescmd])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $abs_srcdir/$1 | getdap4 -D -M - || true], [], [stdout], [stderr])
AT_CHECK([diff -b -B $abs_srcdir/$1.baseline stdout || diff -b -B $abs_srcdir/$1.baseline stderr], [], [ignore],[],[])
AT_CLEANUP])

