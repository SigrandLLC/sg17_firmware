does not compile, just TODO notes

#if 0
	    unsigned char modemstate = 0;

	    if (ioctl(port->devfd, TIOCMGET, &val) != -1)
	    {
		if (val & TIOCM_CD)
		    modemstate |= 0x80;
		if (val & TIOCM_RI)
		    modemstate |= 0x40;
		if (val & TIOCM_DSR)
		    modemstate |= 0x20;
		if (val & TIOCM_CTS)
		    modemstate |= 0x10;

		modemstate &= port->modemstate_mask;
		if (modemstate != port->last_modemstate)
		{
		    // Notify modemstate
		    port->last_modemstate = modemstate;
                    // send ...
		}
	    }
#endif
#if 0
	    port->linestate_mask = 0;
	    port->modemstate_mask = 255;
	    port->last_modemstate = 0;

	    /* send a modemstate notify */
	    if (ioctl(port->devfd, TIOCMGET, &val) != -1) {
		if (val & TIOCM_CD)
		    data[2] |= 0x80;
		if (val & TIOCM_RI)
		    data[2] |= 0x40;
		if (val & TIOCM_DSR)
		    data[2] |= 0x20;
		if (val & TIOCM_CTS)
		    data[2] |= 0x10;
		port->last_modemstate = data[2];
	    }
	    // send ...
#endif

	/* DTR handling */
	    val = TIOCM_DTR;
	    ioctl(port->devfd, TIOCMBIS, &val);
	    goto read_dtr_val;

	    val = TIOCM_DTR;
	    ioctl(port->devfd, TIOCMBIC, &val);
	    goto read_dtr_val;

	read_dtr_val:
	    if (ioctl(port->devfd, TIOCMGET, &val) == -1)
		val = 7;
	    else if (val & TIOCM_DTR)
		val = 8;
	    else
		val = 9;
	    break;

	/* RTS handling */
	    val = TIOCM_RTS;
	    ioctl(port->devfd, TIOCMBIS, &val);
	    goto read_rts_val;

	    val = TIOCM_RTS;
	    ioctl(port->devfd, TIOCMBIC, &val);
	    goto read_rts_val;

	read_rts_val:
	    if (ioctl(port->devfd, TIOCMGET, &val) == -1)
		val = 10;
	    else if (val & TIOCM_RTS)
		val = 11;
	    else
		val = 12;
	    break;

