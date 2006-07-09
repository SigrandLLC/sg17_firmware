#!/usr/bin/haserl
<? title="Debug info: kdb"
   . ../common/header.sh
   . ../lib/misc.sh
   . ../lib/widgets.sh
 ?>

<? printTitle ?>

<? 
    displayFile /etc/hostname
    displayFile /etc/inittab
    displayFile /etc/resolv.conf
    displayFile /etc/network/interfaces
    displayFile /etc/thttpd/thttpd.conf

    displayFile /etc/fstab
    displayFile /etc/hosts
    displayFile /etc/profile
    displayFile /etc/passwd
    displayFile /etc/group
    displayFile /etc/services
    displayFile /etc/protocols

?>


<? . ../common/footer.sh ?>
