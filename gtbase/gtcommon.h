/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_COMMON_H__
#define __GT_COMMON_H__

#include <QtCore/qglobal.h>

#define GT_BEGIN_NAMESPACE namespace Gather {
#define GT_END_NAMESPACE }

#if defined(GT_BUILD_BASE_LIB)
#define GT_BASE_EXPORT Q_DECL_EXPORT
#else
#define GT_BASE_EXPORT Q_DECL_IMPORT
#endif

#if defined(GT_BUILD_VIEW_LIB)
#define GT_VIEW_EXPORT Q_DECL_EXPORT
#else
#define GT_VIEW_EXPORT Q_DECL_IMPORT
#endif

#if defined(GT_BUILD_BACKEND_LIB)
#define GT_BACKEND_EXPORT Q_DECL_EXPORT
#else
#define GT_BACKEND_EXPORT Q_DECL_IMPORT
#endif

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#undef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))

#undef CLAMP
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#endif  /* __GT_COMMON_H__ */
