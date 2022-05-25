/**
 * @file noncopyable.h
 * @brief 申明不可拷贝，赋值
 * @details 禁止拷贝、赋值
 * @author 王华林
 * @date 2022-05-25
 * @version 1.0
 * @copyright  Copyright (c) of Ningbo Geely
 * Automobile Research and Development Co., Ltd. 2022
 *
 */
#pragma once
namespace geely {
namespace net {
    class noncopyable {
    public:
        noncopyable(const noncopyable &) = delete;
        noncopyable &operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };
} // namespace net
} // namespace geely