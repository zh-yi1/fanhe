#ifndef ICFG_H_
#define ICFG_H_

/********************Calibration***************************/

#define uT							     31.25

//3D
#define DEFAULT_NORMAL			        (50 *uT)

#define FIRST_CALIB_NUM					(32-1)
#define MAXNUM_BEFORE_ROLLBACK		    48

#define CALIB_INTERVAL_NUM_DEFAULT		16
#define STANDOFF_CHKNUM_3D				16
#define ONLY_UPDATE_UPPER_LIMIT			(85 * uT)
#define ONLY_UPDATE_LOWER_LIMIT			(18 * uT)
#define WRITE_UPPER_LIMIT				(43 * uT)
#define WRITE_LOWER_LIMIT				(53 * uT)
#define ONLY_UPDATE_DIFFSAMECOUNT		(2)
#define X_Y_MAX_MIN_DIFF				(50 *uT)

#define VD_DIFF_MAX						(5*uT)
#define VD_XY_DIFF_MAX					(5*uT)
#define RD_DIFF_MAX						(5*uT)

#endif /* ICFG_H_ */
