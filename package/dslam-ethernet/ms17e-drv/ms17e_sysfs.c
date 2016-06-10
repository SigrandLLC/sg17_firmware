#include "ms17e_sysfs.h"
#include "ms17e_net.h"
#include "ms17e_main.h"
#include "ms17e_debug.h"
#include "ms17e_si345x.h"

#define to_net_dev(class) container_of(class, struct net_device, class_dev)

static ssize_t show_pwr_source(struct class_device *cdev, char *buf)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct ms17e_card *card=nl->card;

	return snprintf(buf, PAGE_SIZE, "%d", card->pwr_source);
}
static CLASS_DEVICE_ATTR(pwr_source,0444,show_pwr_source,NULL);

static ssize_t show_status(struct class_device *cdev, char *buf)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct ms17e_card *card = nl->card;
	int j = 0, port_status_reg, vee, cur, chip_n, port_n;
	int chip_num = nl->number > 3 ? 2 : 0;
	int port_num = nl->number > 3 ? nl->number - 4 : nl->number;
	int allcurrents = 0;

	port_status_reg = read_poe_reg(chip_num, PORT1_STATUS + port_num, card);
	j += snprintf(&buf[j], PAGE_SIZE, "DET=");
	switch (port_status_reg & 0x7) {
		case 0: j += snprintf(&buf[j], PAGE_SIZE, "'Unknown';\n"); break;
		case 1: j += snprintf(&buf[j], PAGE_SIZE, "'Short';\n"); break;
		case 3: j += snprintf(&buf[j], PAGE_SIZE, "'Low';\n"); break;
		case 4: j += snprintf(&buf[j], PAGE_SIZE, "'Good';\n"); break;
		case 5: j += snprintf(&buf[j], PAGE_SIZE, "'High';\n"); break;
		case 6: j += snprintf(&buf[j], PAGE_SIZE, "'Open';\n"); break;
	}
	j += snprintf(&buf[j], PAGE_SIZE, "CLS=");
	switch ((port_status_reg & 0x38) >> 3) {
		case 0: j += snprintf(&buf[j], PAGE_SIZE, "'Unknown';\n"); break;
		case 1: j += snprintf(&buf[j], PAGE_SIZE, "'class 1';\n"); break;
		case 2: j += snprintf(&buf[j], PAGE_SIZE, "'class 2';\n"); break;
		case 3: j += snprintf(&buf[j], PAGE_SIZE, "'class 3';\n"); break;
		case 4: j += snprintf(&buf[j], PAGE_SIZE, "'class 4';\n"); break;
		case 5: j += snprintf(&buf[j], PAGE_SIZE, "'probes no equal';\n"); break;
		case 6: j += snprintf(&buf[j], PAGE_SIZE, "'class 0';\n"); break;
		case 7: j += snprintf(&buf[j], PAGE_SIZE, "'class overload';\n"); break;
	}
	if (port_status_reg & 0x40) j += snprintf(&buf[j], PAGE_SIZE, "PowerEnable=1;\n");
	if (port_status_reg & 0x80) j += snprintf(&buf[j], PAGE_SIZE, "PowerGood=1;\n");

	write_poe_reg(chip_num, COMMAND_REG, CMD_GET_VEE, card);
	vee = read_poe_reg(chip_num, VEE_LSB, card);
	vee += read_poe_reg(chip_num, VEE_MSB, card) << 8;
	j += snprintf(&buf[j], PAGE_SIZE, "VEE=%i.%03i;\n", vee / 1000, vee % 1000);

//	write_poe_reg(chip_num, COMMAND_REG, CMD_GET_CURRENT | port_num, card);
//	cur = read_poe_reg(chip_num, CUR_P1_LSB + 2 * port_num, card);
//	cur += read_poe_reg(chip_num, CUR_P1_MSB + 2 * port_num, card) << 8;
//	j += snprintf(&buf[j], PAGE_SIZE, "Current=%i.%04i;\n", cur / 10000, cur % 10000);

	for (chip_n = 0; chip_n <= 2; chip_n += 2) {
		for (port_n = 0; port_n < 4; port_n++) {
			write_poe_reg(chip_n, COMMAND_REG, CMD_GET_CURRENT | port_n, card);
			cur = read_poe_reg(chip_n, CUR_P1_LSB + 2 * port_n, card);
			cur += read_poe_reg(chip_n, CUR_P1_MSB + 2 * port_n, card) << 8;
			if ((chip_num == chip_n) && (port_num == port_n)) {
				j += snprintf(&buf[j], PAGE_SIZE, "Current=%i.%04i;\n", cur / 10000, cur % 10000);
			}
			allcurrents += cur;
		}
	}
	j += snprintf(&buf[j], PAGE_SIZE, "AllCurrents=%i.%04i;\n", allcurrents / 10000, allcurrents % 10000);
//	port_i_cut_reg = read_poe_reg(chip_num, PORT1_I_CUT + port_num, card);
//	port_config_reg = read_poe_reg(chip_num, PORT1_CONFIG + port_num, card);

//	j += snprintf(&buf[j], PAGE_SIZE, "status_reg = %02x i_cut_reg = %02x config_reg = %02x\n", port_status_reg, port_i_cut_reg, port_config_reg);

	return j;
}
static CLASS_DEVICE_ATTR(status, 0444, show_status, NULL);

static ssize_t show_config(struct class_device *cdev, char *buf)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local    *nl = netdev_priv(ndev);
//	struct ms17e_card *card = nl->card;
	int j = 0, port_config, port_icut;
//	int chip_num = nl->number > 3 ? POE_CHIP1 : POE_CHIP0;
//	int port_num = nl->number > 3 ? nl->number - 4 : nl->number;

	port_icut = nl->chan_cfg->icut_reg;
	port_config = nl->chan_cfg->config_reg;

//	port_config = read_poe_reg(chip_num, PORT1_CONFIG + port_num, card);
//	port_icut = read_poe_reg(chip_num, PORT1_I_CUT + port_num, card);

	j += snprintf(&buf[j], PAGE_SIZE, "auto=%i;\n", ((port_config & 0x3) == 3) ? 1 : 0);
	j += snprintf(&buf[j], PAGE_SIZE, "current=%i;\n", port_icut * 32 / 10);
	j += snprintf(&buf[j], PAGE_SIZE, "auto_off=%i;\n", (port_config & 0x4) ? 1 : 0);
	j += snprintf(&buf[j], PAGE_SIZE, "poe_plus=%i;\n", (port_config & 0x8) ? 1 : 0);

	return j;
}
/* echo "1 1" - авто с poe+
   echo "1 0" - авто без poe+
   echo "0 [0-5]" - мануал, задание класса (5 - poe+)
*/
static ssize_t store_config( struct class_device *cdev,const char *buf, size_t size )
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct ms17e_card *card = nl->card;
	int chip_num = nl->number > 3 ? POE_CHIP1 : POE_CHIP0;
	int port_num = nl->number > 3 ? nl->number - 4 : nl->number;
	char *endp;
	int auto_conf = 0;
	int poe_class = 0;
	int icut      = 0;
	int auto_off  = 0;
	int poe_plus  = 0;

	if (!size) return 0;

	write_poe_reg(chip_num, COMMAND_REG, CMD_PORT_RESET | port_num, card);
	mdelay(100);
//	write_poe_reg(chip_num, COMMAND_REG, CMD_PORT_POWER_OFF | port_num, card);

	auto_conf = simple_strtoul(buf, &endp, 10);
	poe_plus = simple_strtoul(&endp[1], &endp, 10);
	if (auto_conf)
	{
		// в режиме авто питание включится, поэтому применяем настройки только если надо
		if (nl->chan_cfg->pwr_enable) {
			write_poe_reg(chip_num, PORT1_I_CUT + port_num, 0x75, card);
			write_poe_reg(chip_num, PORT1_CONFIG + port_num, ((0x01 & poe_plus) << 3) | 0x07, card);
		}
		// если питание выключено просто сохраняем настройки в памяти, чтобы когда питание включат мы их применили
		nl->chan_cfg->icut_reg   = 0x75;
		nl->chan_cfg->config_reg = ((0x01 & poe_plus) << 3) | 0x07;
		PDEBUG(0, "auto conf_reg=%02x", nl->chan_cfg->config_reg);
	} else {
		poe_class = poe_plus;
		poe_plus = 0;
		auto_off = 0;
		switch (poe_class) {
			case 0: icut = 0x75; break;
			case 1: icut = 0x1E; break;
			case 2: icut = 0x35; break;
			case 3: icut = 0x75; break;
			case 4: icut = 0x75; break;
			case 5: icut = 0xC9; poe_plus = 1; break;
			default: icut = 0x75; break;
		}
		// в режиме мануал питание нужно включать, поэтому насройки можно применить
		write_poe_reg(chip_num, PORT1_CONFIG + port_num, ((0x01 & poe_plus) << 3) | ((0x01 & auto_off) << 2) | 0x01, card);
		write_poe_reg(chip_num, PORT1_I_CUT + port_num, icut, card);
		if (nl->chan_cfg->pwr_enable) {
			write_poe_reg(chip_num, COMMAND_REG, CMD_PORT_POWER_ON | port_num, card);
		}
		// но сохранить в памяти их надо в любом случае
		nl->chan_cfg->icut_reg = icut;
		nl->chan_cfg->config_reg = ((0x01 & poe_plus) << 3) | ((0x01 & auto_off) << 2) | 0x01;
	}

	PDEBUG(0, "auto_conf = %i poe_plus = %i auto_off = %i icut = %i poe_class = %i", auto_conf, poe_plus, auto_off, icut, poe_class);
	return size;
}
static CLASS_DEVICE_ATTR(config, 0644, show_config, store_config);

static ssize_t store_pwr_enable( struct class_device *cdev,const char *buf, size_t size )
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct ms17e_card *card = nl->card;
	int chip_num = nl->number > 3 ? 2 : 0;
	int port_num = nl->number > 3 ? nl->number - 4 : nl->number;
	char *endp;
	int on_off = 0;

	on_off = simple_strtoul(buf, &endp, 10);

	write_poe_reg(chip_num, COMMAND_REG, CMD_PORT_RESET | port_num, card);
	mdelay(100);
	if (on_off)
	{
		write_poe_reg(chip_num, PORT1_I_CUT + port_num, nl->chan_cfg->icut_reg, card);
		write_poe_reg(chip_num, PORT1_CONFIG + port_num, nl->chan_cfg->config_reg, card);
		if ((nl->chan_cfg->config_reg & 3) != 3) {
		    write_poe_reg(chip_num, COMMAND_REG, CMD_PORT_POWER_ON | port_num, card);
		}
		nl->chan_cfg->pwr_enable=1;
	} else {
		write_poe_reg(chip_num, PORT1_CONFIG + port_num, 0x00, card);
		led_control(chip_num ? 3 - port_num : 7 - port_num, OFF, card);
		nl->chan_cfg->pwr_enable=0;
	}
	return size;
}
static ssize_t show_pwr_enable(struct class_device *cdev, char *buf)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local    *nl = netdev_priv(ndev);
	struct ms17e_card *card = nl->card;
	int port_status_reg;
	int chip_num = nl->number > 3 ? 2 : 0;
	int port_num = nl->number > 3 ? nl->number - 4 : nl->number;

	port_status_reg = read_poe_reg(chip_num, PORT1_STATUS + port_num, card);
	if (nl->chan_cfg->pwr_enable)
		return  snprintf(buf, PAGE_SIZE, "1\n");
	else
		return  snprintf(buf, PAGE_SIZE, "0\n");
}

static CLASS_DEVICE_ATTR(pwr_enable, 0644, show_pwr_enable, store_pwr_enable);

static struct attribute *ms17e_attr[] = {
	&class_device_attr_pwr_source.attr,
	&class_device_attr_status.attr,
	&class_device_attr_config.attr,
	&class_device_attr_pwr_enable.attr,
	NULL
};

static struct attribute_group ms17e_group = {
	.name  = "ms_private",
	.attrs  = ms17e_attr,
};

struct dev_entrie {
	char *name;
	int mark;
	struct proc_dir_entry *pent;
	mode_t mode;
	read_proc_t *fread;
	write_proc_t *fwrite;
};


int ms17e_sysfs_register(struct net_device *ndev)
{
	struct class_device *class_dev = &(ndev->class_dev);
	sysfs_create_group(&class_dev->kobj, &ms17e_group);
	return 0;
}

void ms17e_sysfs_remove(struct net_device *ndev)
{
	struct class_device *class_dev = &(ndev->class_dev);
	sysfs_remove_group(&class_dev->kobj, &ms17e_group);
//	sysfs_remove_link(&(class_dev->kobj),"device");
}
