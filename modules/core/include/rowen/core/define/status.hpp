#pragma once

enum StatusCode
{
  rssSuccess   = 0,  // 성공 상태 코드 (200 OK와 같은 의미)
  rssUndefined = 1,  // 정의되지 않은 상태 코드
  rssException = 2,  // 예외 상태 코드

  // 1xx Informational
  rssProcessing = 102,  // 요청을 처리 중임을 나타내는 상태 코드

  // 2xx Success
  rssOK             = 200,  // 요청이 성공적으로 완료되었음을 나타내는 상태 코드
  rssCreated        = 201,  // 요청이 성공적으로 처리되었으며 새로운 리소스가 생성되었음을 나타내는 상태 코드
  rssAccepted       = 202,  // 요청이 처리 중임을 나타내는 상태 코드
  rssTerminated     = 203,  // 해당하는 리소스가 종료되었음을 나타내는 상태 코드
  rssResetContent   = 205,  // 요청이 성공적으로 처리되었으며 클라이언트는 현재 문서 뷰를 재설정해야 함을 나타내는 상태 코드
  rssPartialContent = 206,  // 서버가 요청의 일부만 성공적으로 처리했음을 나타내는 상태 코드
  rssNotModified    = 207,  // 클라이언트가 마지막 요청 이후에 리소스가 수정되지 않았음을 나타내는 상태 코드
  rssPassButExpired = 208,  // 요청이 성공적으로 처리되었으나 일부 리소스가 만료되었음을 나타내는 상태 코드
  rssInUseResource  = 226,  // 요청한 리소스가 이미 사용 중임을 나타내는 상태 코드

  // 3xx Warnings
  // rssInUseResource = 300,  // 요청한 리소스가 이미 사용 중임을 나타내는 상태 코드

  // 4xx Client errors
  rssBadRequest                  = 400,  // 클라이언트 요청이 잘못되었음을 나타내는 상태 코드
  rssUnauthorized                = 401,  // 인증이 필요함을 나타내는 상태 코드
  rssInvalidParameter            = 402,  // 요청 매개변수가 잘못되었음을 나타내는 상태 코드
  rssNotFound                    = 404,  // 요청한 명령이나 리소스를 찾을 수 없음을 나타내는 상태 코드
  rssConflict                    = 409,  // 요청이 현재 상태와 충돌함을 나타내는 상태 코드
  rssLengthRequired              = 411,  // Content-Length 헤더 필드가 누락되었음을 나타내는 상태 코드
  rssInvalidPacket               = 412,  // 요청 패킷이 잘못되었음을 나타내는 상태 코드
  rssInvalidPayload              = 413,  // 요청 페이로드가 너무 크거나 작음을 나타내는 상태 코드
  rssRangeNotSatisfiable         = 416,  // 요청 범위가 만족할 수 없음을 나타내는 상태 코드
  rssLocked                      = 423,  // 리소스가 잠겨 있음을 나타내는 상태 코드
  rssUpgradeRequired             = 426,  // 업그레이드가 필요함을 나타내는 상태 코드
  rssTooManyRequests             = 429,  // 요청이 너무 많음을 나타내는 상태 코드
  rssRequestHeaderFieldsTooLarge = 431,  // 요청 헤더 필드가 너무 큼을 나타내는 상태 코드

  // 5xx Server errors
  rssProgressError        = 500,  // 서버에서 요청을 처리하는 중에 오류가 발생함을 나타내는 상태 코드
  rssNotAllocated         = 501,  // 요청을 처리하는 과정에서 리소스가 할당되지 않음을 나타내는 상태 코드
  rssNotAvailable         = 502,  // 요청을 처리하는 과정에서 리소스를 사용할 수 없음을 나타내는 상태 코드
  rssNotSupported         = 503,  // 요청을 처리하는 과정에서 리소스를 지원하지 않음을 나타내는 상태 코드
  rssProcessTimeout       = 504,  // 서버에서 요청을 처리하는 중에 시간 초과가 발생함을 나타내는 상태 코드
  rssProtocolNotSupported = 505,  // 서버가 요청된 HTTP 버전을 지원하지 않음을 나타내는 상태 코드
  rssInsufficientStorage  = 507,  // 서버가 저장 공간이 부족함을 나타내는 상태 코드
};

namespace rs {

inline constexpr bool rss_global_state_is_success(int status_code, bool strict = true)
{
  if (status_code == rssSuccess)
    return true;

  const auto status_group = status_code / 100;

  if (strict)
    return status_group == 2;
  else
    return status_group == 1 || status_group == 2 || status_group == 3;
}

inline constexpr const char* rss_global_state_string(int status_code)
{
  switch (status_code)
  {
    case rssSuccess:                     return "The request was successfully completed.";
    case rssUndefined:                   return "Undefined status code.";
    case rssException:                   return "An exception occurred.";

    case rssProcessing:                  return "The request is being processed.";

    case rssOK:                          return "The request was successfully completed.";
    case rssCreated:                     return "The request was successfully processed, and a new resource was created.";
    case rssAccepted:                    return "The request has been accepted and is being processed.";
    case rssResetContent:                return "The request was completed, and the client should reset the current document view.";
    case rssPartialContent:              return "The server successfully processed part of the request.";
    case rssNotModified:                 return "The client's request has not been modified since the last request.";
    case rssPassButExpired:              return "The request was successfully processed, but some resources have expired.";
    case rssTerminated:                  return "The corresponding resource has been terminated.";

    case rssInUseResource:               return "The requested resource is already in use.";

    case rssBadRequest:                  return "The request is invalid or malformed.";
    case rssUnauthorized:                return "Authentication is required.";
    case rssInvalidParameter:            return "The request parameters are invalid.";
    case rssNotFound:                    return "The requested resource or command was not found.";
    case rssConflict:                    return "The request conflicts with the current state.";
    case rssLengthRequired:              return "Content-Length header is missing.";
    case rssInvalidPacket:               return "The request packet is invalid.";
    case rssInvalidPayload:              return "The request payload is too large or short.";
    case rssRangeNotSatisfiable:         return "The request range cannot be satisfied.";
    case rssLocked:                      return "The resource is locked.";
    case rssUpgradeRequired:             return "Upgrade is required.";
    case rssTooManyRequests:             return "Too many requests.";
    case rssRequestHeaderFieldsTooLarge: return "Request header fields are too large.";

    case rssProgressError:               return "A server error occurred while processing the request.";
    case rssNotAllocated:                return "Resource allocation failed while processing the request.";
    case rssNotAvailable:                return "The resource is not available to process the request.";
    case rssNotSupported:                return "The resource does not support the request.";
    case rssProcessTimeout:              return "The server timed out while processing the request.";
    case rssProtocolNotSupported:        return "The server does not support the requested HTTP version.";
    case rssInsufficientStorage:         return "The server is out of storage space.";

    default:                             return "Unknown status code.";
  }
}

};  // namespace rs
