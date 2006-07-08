#!/usr/bin/haserl
<? 
	. ../conf/conf.sh
	title="Debug info: kdb"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
 ?>

<? printTitle ?>

<table>
<? printTableTitle "kdb list" ?>
<tr><td>

<pre class="code">
<?
   kdb list
?>
</pre>
</td></tr>
</table>


<? . ../common/footer.sh ?>
