/**
 * @Code	L10N (Localization) By JavaScript
 * @Desc	This is a very useful script based on JavaScript and XML. You can use it to the localization of the webpage.
 * @Version	1.0 build 20080413
 * @Author	Hpyer <hpyer@yahoo.cn>
 * @Home	http://www.hpyer.cn/codes/l10n-by-javascript
 * @Usage	<See docs/ReadMe.txt>
 * @License	GNU GENERAL PUBLIC LICENSE (GPL) <See docs/License.txt>
 * @History	<See docs/History.txt>
 */

(function($) {

	$.extend({
		l10n: {
			/**
			 * @desc	Options
			 *				'dir': Path to the folder of language files
			 *				'lang': Current language
			 *				'cache': If set to false it will force the pages that you request to not be cached by the browser
			 * @type	Map
			 * @default	{'dir': './languages', 'lang': 'en', 'cache': true}
			 * @access	private
			 */
			opts: {
				'dir': './languages',
				'lang': 'en',
				'cache': true
			},

			/**
			 * @desc	Language of the language file
			 * @type	String
			 * @access	private
			 */
			lang_msg: '',

			/**
			 * @desc	Store all messages that read from language file
			 * @type	Map
			 * @access	private
			 */
			messages: [],

			/**
			 * @desc	Translate the words (Translate directly)
			 * @param	String	words	Words need to be translate
			 * @access	public
			 * @access	string
			 * @usage	$.l10n.__('Words need to be translated');
			 */
			__: function(words) {
				if (!this.check()) {
					return words;
				}
				return this.messages[this.opts['lang']][words];
			},	// end __

			/**
			 * @desc	Translate and output
			 * @param	String	words	Words need to be translated
			 * @access	public
			 * @access	void
			 * @usage	$.l10n._e('Words need to be translated');
			 */
			_e: function(words) {
				document.write(this.__(words));
				return '';
			},	// end _e

			/**
			 * @desc	Check whether to need to translate or not
			 * @access	public
			 * @access	boolean
			 */
			check: function() {
				if (!this.opts['lang'] || this.opts['lang'] == 'en'){
					return false;
				}
				if (!this.messages[this.opts['lang']]) {
					this.load();
				}
				return true;
			},	// end check

			/**
			 * @desc	Initialization options
			 * @param	Map	opts	The same to $.l10n.opts
			 * @access	public
			 * @return	void
			 */
			init: function(opts) {
				if (typeof(opts) != 'object') {
					return false;
				}
				for (opt in opts) {
					if (opts[opt]) {
						this.opts[opt] = opts[opt];
					}
				}
			},	// end init

			/**
			 * @desc	Load the language file
			 * @param	String	file	Filename of the language file
			 * @access	private
			 * @return	int	0: Failure, 1: Success, 2: Language was loaded
			 */
			load: function() {
				var uri = this.opts['dir'] + '/' + this.opts['lang'] + '.xml';
				$.ajax({
					'url': uri,
					'cache': (this.opts['cache'] ? true : false),
					'async': false,
					'success': function(data) {
						$.l10n.callback(data);
					}
				});	// end $.ajax
			},	// end load

			/**
			 * @desc	Read all messages from the language file
			 * @param	Object	root	Root of XML document
			 * @access	private
			 * @return	void
			 */
			callback: function(root) {
				if (!root) {
					return false;
				}
				$.l10n.lang_msg = $.l10n.opts['lang'];
				$.l10n.messages[$.l10n.opts['lang']] = [];
				var msgs = $('message', root);
				$.each(msgs, function(){
					var msgid, msgstr;
					try {
						msgid = $('msgid', this)[0].firstChild.nodeValue;
						msgstr = $('msgstr', this)[0].firstChild.nodeValue;
						if ($.browser.msie) {
							// work still with bugs for IE
							//var res = msgid.match(/<(.+)(\s[^>]*)*>([^<]*)<\/\1>/g);
							var res = msgid.match(/<(.+)>([^<]*)<\/\1>/g);
							if (res) {
								$.each(res, function() {
									var arr = /<(.+)>([^<]*)<\/\1>/g.exec(this);
									var tag = arr[1].toUpperCase();
									msgid = msgid.replace(new RegExp(arr[0], 'g'), '<'+ tag +'>'+ arr[2] +'</'+ tag +'>');
								});
							}
						}	// end if ($.browser.msie)
					} catch (e) {alert(e.message);}	// end try
					if (msgid && msgstr) {
						$.l10n.messages[$.l10n.opts['lang']][msgid] = msgstr;
					}
				});	// end $.each
			}	// end callback

		}	// end l10n
	});	// end $.extend

	$.fn.extend({
		/**
		 * @desc	Translate all messages that was selected by jQuery.selector (Translate indirectly)
		 * @param	Map	opts	The same to $.l10n.opts
		 * @access	public
		 * @access	void
		 * @usage	$(selector).l10n();
		 */
		l10n: function(opts) {
			$.l10n.init(opts);
			this.each(function() {
				var msgid = $(this).attr('msgid');
				if (!msgid) {
					msgid = $(this).html();
					$(this).attr('msgid', msgid);
				}
				$(this).html($.l10n.__(msgid));
			});
		},	// end l10n

		/**
		 * @desc	Pick all messages that was selected by jQuery.selector
		 * @param	String	encode	Encode of XML file
		 * @param	Boolean	parse_func	If set to true it will parse functions `_e` and `__`
		 * @access	public
		 * @access	void
		 * @usage	$(selector).l10n_picker();
		 */
		l10n_picker: function(encode, parse_func) {

			function parse(html, reg) {
				var re = new RegExp(reg, 'gi');
				var arr = html.match(re);
				if (arr && arr.length > 0) {
					for (var i=0, l=arr.length; i<l; i++) {
						var res = new RegExp(reg, 'gi').exec(arr[i]);
						if (res[2] && !isCached(res[2])) {
							msgs.push(res[2]);
						}
					}
				}
			}	// end parse

			function toXml(encode) {
				var str = '<?xml version="1.0" encoding="' + encode + '"?>\n\n<messages>\n';
				for (var i=0, l=msgs.length; i<l; i++) {
					str += '\t<message>\n'
						+  '\t\t<msgid><![CDATA[' + msgs[i] + ']]></msgid>\n'
						+  '\t\t<msgstr><![CDATA[]]></msgstr>\n'
						+  '\t</message>\n';
				}
				str += '</messages>';
				return str;
			}	// end toXml

			function isCached(msgid) {
				for (var i=0, l=msgs.length; i<l; i++) {
					if (msgs[i] == msgid) {
						return true;
					}
				}
				return false;
			}	// end isCached

			if (!encode) {
				encode = 'UTF-8';
			}
			parse_func = parse_func || false;
			var msgs = [];

			// for `$(obj).l10n()`
			this.each(function() {
				var msgid = $(this).attr('msgid');
				if (!msgid) {
					msgid = $(this).html();
					$(this).attr('msgid', msgid);
				}
				if (msgid && !isCached(msgid)) {
					msgs.push(msgid);
				}
			});
			if (parse_func) {
				var html = $('html').html();
				// for function `$.l10n._e()`
				parse(html, '_e\\((\"|\'){1}([^\)]*)\\1(,\s*.+)*\\)');
				// for function `$.l10n.__()`
				parse(html, '_{2}\\((\"|\'){1}([^\)]*)\\1(,\s*.+)*\\)');
			}

			return toXml(encode);
		}	// end l10n_picker
	});	// end $.fn.extend

})(jQuery);