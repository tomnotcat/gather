/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_LINK_DEST_H__
#define __GT_LINK_DEST_H__

#include "gtobject.h"

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtLinkDest : public GtObject
{
public:
    enum LinkType {
        LinkNone,
        LinkGoto
    };

public:
    explicit GtLinkDest();
    explicit GtLinkDest(int page);
    ~GtLinkDest();

public:
    inline LinkType type() const { return m_type; }
    inline int page() const { return m_data.gotor.page; }

public:
    LinkType m_type;
    union {
        struct {
            int page;
        }
        gotor;
    } m_data;
};

GT_END_NAMESPACE

#endif  /* __GT_LINK_DEST_H__ */
