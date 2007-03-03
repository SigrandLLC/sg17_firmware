#!/usr/bin/haserl

	page=${FORM_page:-backupcfg}
	
	render_page_selection "" backupcfg "backupcfg" restorecfg "restorecfg"

	render_form_header backup "action='/cfg.cgi'" 
	render_table_title "$page" 2 

	case $page in
	backupcfg)
		render_form_header backup "action='/cfg.cgi'" 
		render_input_field hidden act act "backup"
		render_submit_field "Backup"
	;;
	restorecfg)
		render_input_field hidden act act "restore"
		desc="Restore configuration from file"
		render_input_field file "Restore configuration" "uploadfile"
		render_submit_field "Restore"
	;;
	esac
	render_form_tail

