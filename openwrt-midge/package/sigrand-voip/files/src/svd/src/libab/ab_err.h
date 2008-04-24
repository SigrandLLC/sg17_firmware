#ifndef __AB_ERR_H__
#define __AB_ERR_H__

#define ab_err_set(objp, err_idx, str)			\
	do {						\
		ab_g_err_str = (str); 			\
		ab_g_err_idx = (err_idx);		\
		if(objp) {				\
			(objp)->err_s = (str);		\
			(objp)->err = (err_idx);	\
		}					\
	} while(0)

#endif /* __AB_ERR_H__ */
