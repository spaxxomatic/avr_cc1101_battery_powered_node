#ifndef _REGTABLE_H
#define _REGTABLE_H

#include "product.h"
#include "register.h"
#include "commonregs.h"

/**
 * Register indexes
 */
DEFINE_REGINDEX_START()
  REGI_VOLTSUPPLY,
  REGI_SETGATE,
  REGI_SETLIGHT
DEFINE_REGINDEX_END()

#endif
