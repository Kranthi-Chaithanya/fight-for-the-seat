// Inlines the single-file Emscripten output into the web page, producing one HTML file.
// Usage: node make-single.js <page.html> <game.js> <out.html>
const fs = require('fs');
const [page, js, out] = process.argv.slice(2);
let html = fs.readFileSync(page, 'utf8');
const code = fs.readFileSync(js, 'utf8').replace(/<\/script/gi, '<\\/script');
const tag = '<script src="game.js"></script>';
if (!html.includes(tag)) { console.error('game.js script tag not found'); process.exit(1); }
html = html.replace(tag, () => '<script>\n' + code + '\n</script>');
if (process.argv[5] !== '--bare' && !/^<!doctype/i.test(html)) html = '<!doctype html>\n<html lang="en">\n<meta charset="utf-8">\n<meta name="viewport" content="width=device-width, initial-scale=1">\n' + html;
fs.writeFileSync(out, html);
console.log('wrote ' + out + ' (' + (html.length / 1048576).toFixed(2) + ' MB)');
