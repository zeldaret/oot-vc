#ifndef RVL_SDK_HBM_CONFIG_H
#define RVL_SDK_HBM_CONFIG_H

/*******************************************************************************
 * options
 */

/* HBM library version used by DVD games.
 */
#define HBM_APP_TYPE_DVD 1

/* HBM library version used by NAND games (WiiWare, VC). Contains a few more
 * functions, types, and members unseen in symbol maps and debug info for DVD
 * games.
 */
#define HBM_APP_TYPE_NAND 2

/*******************************************************************************
 * configuration
 */

/* The macro HBM_APP_TYPE must be defined to one of these two values.
 *
 * You can use one of the defines here or set HBM_APP_TYPE in your build system.
 */
// #define HBM_APP_TYPE	HBM_APP_TYPE_DVD
// #define HBM_APP_TYPE	HBM_APP_TYPE_NAND

#ifndef HBM_APP_TYPE
#error HBM_APP_TYPE was not configured. See HBMConfig.h for details.
#endif

#endif // RVL_SDK_HBM_CONFIG_H
