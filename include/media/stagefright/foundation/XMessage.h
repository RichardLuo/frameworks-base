/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef X_MESSAGE_H_

#define X_MESSAGE_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <vector>
#include <string>

namespace android {

struct Parcel;
struct AString;
class String8;

struct XMessage : public RefBase {
    XMessage(uint32_t what = 0);

    static sp<XMessage> fromParcel(const Parcel &parcel);
    void writeToParcel(Parcel *parcel) const;

    void setWhat(uint32_t what);
    uint32_t what() const;

    void clear();

    void setInt32(const char *name, int32_t value);
    void setInt64(const char *name, int64_t value);
    void setUInt32(const char *name, uint32_t value) {
        setInt32(name, (int32_t)value);
    }
    void setUInt64(const char *name, uint64_t value) {
        setInt64(name, (int64_t)value);
    }
    void setSize(const char *name, size_t value);
    void setFloat(const char *name, float value);
    void setDouble(const char *name, double value);
    void setPointer(const char *name, void *value);
    void setString8(const char *name, const String8 &str);
    void setStdString(const char *name, const std::string &str);
    void setString(const char *name, const char *s, ssize_t len = -1);
    void setObject(const char *name, const sp<RefBase> &obj);
    void setMessage(const char *name, const sp<XMessage> &obj);
    void setStdVector(const char *name, const std::vector<int32_t> &vec);

    void setRect(const char *name,
                 int32_t left, int32_t top, int32_t right, int32_t bottom);

    bool findInt32(const char *name, int32_t *value) const;
    bool findInt64(const char *name, int64_t *value) const;
    bool findUInt32(const char *name, uint32_t *value) const {
        return findInt32(name, (int32_t*)value);
    }
    bool findUInt64(const char *name, uint64_t *value) const {
        return findInt64(name, (int64_t*)value);
    }
    bool findSize(const char *name, size_t *value) const;
    bool findFloat(const char *name, float *value) const;
    bool findDouble(const char *name, double *value) const;
    bool findPointer(const char *name, void **value) const;
    bool findString(const char *name, AString *value) const;
    bool findString8(const char *name, String8 *value) const;
    bool findObject(const char *name, sp<RefBase> *obj) const;
    bool findMessage(const char *name, sp<XMessage> *obj) const;

    bool findStdString(const char *name, std::string *value) const;
    bool findStdVector(const char *name, std::vector<int32_t> *vec) const;

    bool findRect(const char *name,
                  int32_t *left, int32_t *top, int32_t *right, int32_t *bottom) const;

    // If this returns true, the sender of this message is synchronously
    // awaiting a response, the "replyID" can be used to send the response
    // via "postReply" below.
    bool senderAwaitsResponse(uint32_t *replyID) const;

    // Performs a deep-copy of "this", contained messages are in turn "dup'ed".
    // Warning: RefBase items, i.e. "objects" are _not_ copied but only have
    // their refcount incremented.
    sp<XMessage> dup() const;

    AString debugString(int32_t indent = 0) const;

protected:
    virtual ~XMessage();

private:
    enum Type {
        kTypeInt32,
        kTypeInt64,
        kTypeSize,
        kTypeFloat,
        kTypeDouble,
        kTypePointer,
        kTypeString,
        kTypeObject,
        kTypeMessage,
        kTypeRect,
        kTypeStdVector,
        kTypeStdString,
    };

    uint32_t mWhat;

    struct Rect {
        int32_t mLeft, mTop, mRight, mBottom;
    };

    struct Item {
        union {
            int32_t int32Value;
            int64_t int64Value;
            size_t sizeValue;
            float floatValue;
            double doubleValue;
            void *ptrValue;
            RefBase *refValue;
            AString *stringValue;
            Rect rectValue;
            int32_t *int32ArrayValue;
            std::string *stdStringValue;
        } u;
        const char *mName;
        Type mType;
    };

    enum {
        kMaxNumItems = 16
    };
    Item mItems[kMaxNumItems];
    size_t mNumItems;

    Item *allocateItem(const char *name);
    void freeItem(Item *item);
    const Item *findItem(const char *name, Type type) const;

    DISALLOW_EVIL_CONSTRUCTORS(XMessage);
};

}  // namespace android

#endif  // X_MESSAGE_H_
