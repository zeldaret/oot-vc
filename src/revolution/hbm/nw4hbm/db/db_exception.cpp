#include "revolution/hbm/nw4hbm/db/exception.hpp"

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/db/console.hpp"

/*******************************************************************************
 * types
 */

namespace nw4hbm {
namespace db {
struct ExceptionCallbackParam;
}
} // namespace nw4hbm

/*******************************************************************************
 * local function declarations
 */

namespace nw4hbm {
namespace db {
/* Name: [RYWE01]/map/RevoYawarakaD.map:86234,86323
 * For weak function ordering on debug
 */
void DumpException_(ExceptionCallbackParam const*);

/* Name: [RYWE01]/map/RevoYawarakaD.map:86324,86326
 * For weak function ordering on debug
 */
void DrawConsoleEndless_(detail::ConsoleHead*);
} // namespace db
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace db {

void DumpException_(ExceptionCallbackParam const*) { Console_SetVisible(nullptr, false); }

void DrawConsoleEndless_(detail::ConsoleHead*) { Console_SetViewBaseLine(nullptr, 0); }

} // namespace db
} // namespace nw4hbm
