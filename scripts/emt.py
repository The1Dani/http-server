# https://svn.apache.org/repos/asf/httpd/httpd/trunk/docs/conf/mime.types
import re

start = """#ifndef MTYPES
#define MTYPES

#include "mime.h"
"""
middle = """
const struct mime_type_t MIME_TYPES[MIME_TYPE_COUNT] = {
"""

end = """
};

#endif
"""


lines: list[str]
with open("mime.types", "r") as ts:
    lines = ts.readlines()

lines = list(map(str.strip, lines))
lines = [l + "\n" for l in lines if l and not l.startswith("#")]
lines = [re.sub(r"\s+", " ", l) + "\n" for l in lines]

new_lines = []
for words in map(str.split, lines):
    mime, *exts = words
    for ext in exts:
        new_lines.append((mime, ext))

new_lines = sorted(new_lines, key= lambda a: a[1])

c_lib = []
c_lib.append(start)
c_lib.append(f"""
#define MIME_TYPE_COUNT {len(new_lines)}
""")
c_lib.append(middle)
for mime, ext in new_lines:
    c_lib.append(f"\t{{.ext = \"{ext}\", .mime = \"{mime}\"}},\n")

c_lib.append(end)

with open("header.out", "w") as emt:
    emt.writelines(c_lib)




