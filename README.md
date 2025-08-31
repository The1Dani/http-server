# HOW TO BUILD

You need to have build-essentials
```bash
make && ./nob
```

## HOW TO RUN
```bash
./listener
```
Go to
```sh
http://localhost:8080/
```

### TODO:
---

Add some functionality similar to

```bash
list-directory() {
	local d=$1

	shopt -s nullglob dotglob

	echo '<!DOCTYPE html>'
	echo '<html lang="en">'
	echo '<head>'
	echo '  <meta charset="utf-8">'
	printf '  <title>Index of %s</title>\n' "$(html-encode "$d")"
	echo '  <style>'
	echo '  body {'
	echo '    background-color: Canvas;'
	echo '    color: CanvasText;'
	echo '    color-scheme: light dark;'
	echo '  }'
	echo '  a, a:visited, a:active {'
	echo '    text-decoration: none;'
	echo '  }'
	echo '  </style>'
	echo '</head>'
	echo '<body>'
	echo '<h1>Directory Listing</h1>'
	echo "<h2>Directory: $(html-encode "$d")</h2>"
	echo '<hr>'
	echo '<ul>'
	local f
	# loop directories first (to put at top of list)
	for f in .. "$d"/*/; do
		local bname=${f%/}
		bname=${bname##*/}

		local display_name="📁 $bname/"
		printf '<li><a href="%s">%s</a></li>\n' \
			"$(urlencode "$bname")" \
			"$(html-encode "$display_name")"
	done
	# loop regular files next (non-directories)
	for f in "$d"/*; do
		[[ -f $f ]] || continue
		local bname=${f##*/}

		local display_name="📄 $bname"
		printf '<li><a href="%s">%s</a></li>\n' \
			"$(urlencode "$bname")" \
			"$(html-encode "$display_name")"
	done
	echo '</ul>'
	echo '<hr>'
	echo '</body>'
	echo '</html>'
}
```