/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RESULT_H
#define PANDA_RESULT_H

#include <optional>
#include <functional>

namespace panda::es2panda::util {

/**
 * Class to handle errors
 * @tparam SuccessT type in case of success
 * @tparam ErrorStatus error code, should be enum
 * @tparam ErrorV error value to provide detail about error
 * NOTE: `ErrorV = void` not allowed yet.
 */
template <typename SuccessT, typename ErrorStatus, typename ErrorV>
class Result {
    using ErrorT = std::pair<ErrorStatus, ErrorV>;

    class Tag {};
    template <typename... Args>
    explicit Result(Tag /*tag*/, Args... args) : value_ {SuccessT {std::forward<Args>(args)...}}
    {
    }

public:
    Result() = default;

    template <typename... Args>
    static Result Ok(Args... args)
    {
        return Result(Tag {}, std::forward<Args>(args)...);
    }

    static Result Error(ErrorStatus status, ErrorV value)
    {
        return Result(std::make_pair(status, value));
    }

    template <typename OrigValue>
    static Result Error(Result<OrigValue, ErrorStatus, ErrorV> result)
    {
        return Result(result.ExpectError());
    }

    Result(ErrorStatus status, ErrorV errorVal) : value_(std::make_pair(std::move(status), std::move(errorVal))) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(std::pair<ErrorStatus, ErrorV> error) : value_(std::move(error)) {}

    explicit Result(SuccessT success) : value_(success) {}

    explicit operator bool() const
    {
        return IsOk();
    }

    SuccessT &operator*()
    {
        return std::get<SuccessT>(value_);
    }

    const SuccessT &operator*() const
    {
        return std::get<SuccessT>(value_);
    }

    SuccessT *operator->()
    {
        return &std::get<SuccessT>(value_);
    }

    const SuccessT *operator->() const
    {
        return &std::get<SuccessT>(value_);
    }

    bool IsOk() const
    {
        return std::holds_alternative<SuccessT>(value_);
    }

    bool IsError() const
    {
        return !IsOk();
    }

    const std::pair<ErrorStatus, ErrorV> &ExpectError() const
    {
        return std::get<ErrorT>(value_);
    }

    const SuccessT &ExpectOk() const &
    {
        return std::get<SuccessT>(value_);
    }

    SuccessT ExpectOk() &&
    {
        return std::move(std::get<SuccessT>(value_));
    }

    template <typename T>
    Result<T, ErrorStatus, ErrorV> Transform(std::function<T(const SuccessT &)> cb)
    {
        Result<T, ErrorStatus, ErrorV> result;
        if (IsError()) {
            result = Result<T, ErrorStatus, ErrorV>::Error(this);
        } else {
            result = Result<T, ErrorStatus, ErrorV>::Ok(cb(ExpectOk()));
        }
        return result;
    }

    template <typename T>
    Result<T, ErrorStatus, ErrorV> Transform(std::function<T(SuccessT &&)> cb) &&
    {
        using ReturnT = Result<T, ErrorStatus, ErrorV>;
        ReturnT result;
        if (IsError()) {
            result = ReturnT::Error(*this);
        } else {
            result = ReturnT::Ok(cb(std::move(*this).ExpectOk()));
        }
        return result;
    }

private:
    std::variant<SuccessT, ErrorT> value_;
};

}  // namespace panda::es2panda::util

#endif  // PANDA_RESULT_H
