Code	L10N (Localization) By JavaScript
Desc	This is a very useful script based on JavaScript and XML. You can use
	it to the localization of the webpage.
Author	Hpyer <hpyer@yahoo.cn>
Home	http://www.hpyer.cn/codes/l10n-by-javascript
License	GNU GENERAL PUBLIC LICENSE (GPL) <See License.txt>
History	<See History.txt>


[USAGE]

1. Include the script.
<script src="jquery.js" language="JavaScript" type="text/javascript"></script>
<script src="jquery.l10n.js" language="JavaScript" type="text/javascript"></script>

2. Load language file (XML file).
<script type="text/javascript">
$.l10n.init({
	'dir': 'languages',
	'lang': lang,
	'cache': true
});
</script>

3. Translate. There are two ways to do this.
Translate directly
<script type="text/javascript">
document.write($.l10n.__('Words need to be translated'));
</script>

Translate indirectly 
First define a sentence need to be translated with HTML tag span. 
<span domain="l10n">Words need to be translated.</span>
And then call function T_ 
<script type="text/javascript">
L10N.T_('l10n');	// Same to the domain attribute of span
</script>

4. A simple example 
<html>
<head>
<title>A simple example</title>
<script src="jquery.js" language="JavaScript" type="text/javascript"></script>
<script src="jquery.l10n.js" language="JavaScript" type="text/javascript"></script>
<script type="text/javascript">
$.l10n.init({
	'dir': 'languages',
	'lang': lang,
	'cache': true
});
</script>
</head>

<body>
<script type="text/javascript">
// Translate directly
document.write($.l10n.__('Words need to be translated'));
</script>

<!-- Translate indirectly -->
<span>Words need to be translated</span>
<script type="text/javascript">
$('span').l10n();
</script>
</body>
</html>


[PRECAUTIONS]

1. If you want to translate the sentence before the webpage finish loading, you
   must load language file before transfering. The better way, do it between 
   <head> and </head>.
2. The msgid is case sensitive.
3. This code need the library of jQuery.
