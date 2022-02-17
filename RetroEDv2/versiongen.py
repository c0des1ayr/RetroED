from datetime import datetime
import pytz, sys

buildadd = "dev"
buildtype = 0

try:
    buildadd = sys.argv[1]
except: pass
if buildadd != "dev":
    if len(buildadd) >= 6: #autobuild commit hash
        buildtype = 2
        buildadd = buildadd[:6]
    else: buildtype = 1

time = datetime.now(pytz.timezone("US/Eastern"))
s = time.strftime("v%Y.%m.%d-") + buildadd

LINES = [
    "#ifndef VERSION_H\n",
    "#define VERSION_H\n",
   f"#define RE_VERSION    (\"{s}\")\n",
   f"#define RE_BUILD_TYPE ({buildtype})\n",
    "#endif" 
]

open("version.hpp", "w").writelines(LINES)