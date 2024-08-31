#pragma once

namespace chx::sql::postgresql::detail {
enum ParseResult {
    ParseSuccess,

    ParseNeedMore,

    ParseMalformed,
    ParseMalformedIncomplete,
    ParseMalformedTrailingData,

    ParseInternalError
};
}  // namespace chx::sql::postgresql::detail