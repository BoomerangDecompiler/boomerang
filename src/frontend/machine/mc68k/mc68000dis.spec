#
# Copyright (C) 1996, Princeton University
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

address type is "Addr"
address add using "%a + %o"
address to integer using "%a"
fetch any using "getWord(%a)"
