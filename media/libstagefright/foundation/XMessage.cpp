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

#include "XMessage.h"

#include <ctype.h>

#include "AAtomizer.h"
#include "ADebug.h"
#include "ALooperRoster.h"
#include "AString.h"

#include <binder/Parcel.h>
#include <utils/String8.h>

namespace android {

XMessage::XMessage(uint32_t what)
    : mWhat(what),
      mNumItems(0) {
}

XMessage::~XMessage() {
    clear();
}

void XMessage::setWhat(uint32_t what) {
    mWhat = what;
}

uint32_t XMessage::what() const {
    return mWhat;
}

void XMessage::clear() {
    for (size_t i = 0; i < mNumItems; ++i) {
        Item *item = &mItems[i];
        freeItem(item);
    }
    mNumItems = 0;
}

void XMessage::freeItem(Item *item) {
    switch (item->mType) {
        case kTypeString:
        {
            delete item->u.stringValue;
            break;
        }
        case kTypeStdVector:
        {
            delete[] item->u.int32ArrayValue;
            break;
        }
        case kTypeObject:
        case kTypeMessage:
        {
            if (item->u.refValue != NULL) {
                item->u.refValue->decStrong(this);
            }
            break;
        }

        default:
            break;
    }
}

XMessage::Item *XMessage::allocateItem(const char *name) {
    name = AAtomizer::Atomize(name);

    size_t i = 0;
    while (i < mNumItems && mItems[i].mName != name) {
        ++i;
    }

    Item *item;

    if (i < mNumItems) {
        item = &mItems[i];
        freeItem(item);
    } else {
        CHECK(mNumItems < kMaxNumItems);
        i = mNumItems++;
        item = &mItems[i];

        item->mName = name;
    }

    return item;
}

const XMessage::Item *XMessage::findItem(
        const char *name, Type type) const {
    name = AAtomizer::Atomize(name);

    for (size_t i = 0; i < mNumItems; ++i) {
        const Item *item = &mItems[i];

        if (item->mName == name) {
            return item->mType == type ? item : NULL;
        }
    }

    return NULL;
}

#define BASIC_TYPE(NAME,FIELDNAME,TYPENAME)                             \
void XMessage::set##NAME(const char *name, TYPENAME value) {            \
    Item *item = allocateItem(name);                                    \
                                                                        \
    item->mType = kType##NAME;                                          \
    item->u.FIELDNAME = value;                                          \
}                                                                       \
                                                                        \
bool XMessage::find##NAME(const char *name, TYPENAME *value) const {    \
    const Item *item = findItem(name, kType##NAME);                     \
    if (item) {                                                         \
        *value = item->u.FIELDNAME;                                     \
        return true;                                                    \
    }                                                                   \
    return false;                                                       \
}

BASIC_TYPE(Int32,int32Value,int32_t)
BASIC_TYPE(Int64,int64Value,int64_t)
BASIC_TYPE(Size,sizeValue,size_t)
BASIC_TYPE(Float,floatValue,float)
BASIC_TYPE(Double,doubleValue,double)
BASIC_TYPE(Pointer,ptrValue,void *)

#undef BASIC_TYPE

void XMessage::setString(
        const char *name, const char *s, ssize_t len) {
    Item *item = allocateItem(name);
    item->mType = kTypeString;
    item->u.stringValue = new AString(s, len < 0 ? strlen(s) : len);
}

void XMessage::setStdVector(const char *name, const std::vector<int32_t> &vec) {
    Item *item = allocateItem(name);
    item->mType = kTypeStdVector;
    item->u.int32ArrayValue = new int32_t[vec.size() + 1];
    item->u.int32ArrayValue[0] = vec.size();
    for (size_t i = 0; i < vec.size(); i++) {
        item->u.int32ArrayValue[i + 1] = vec[i];
    }
}

void XMessage::setString8(const char *name, const String8 &str) {
    setString(name, str.string());
}

void XMessage::setObject(const char *name, const sp<RefBase> &obj) {
    Item *item = allocateItem(name);
    item->mType = kTypeObject;

    if (obj != NULL) { obj->incStrong(this); }
    item->u.refValue = obj.get();
}

void XMessage::setMessage(const char *name, const sp<XMessage> &obj) {
    Item *item = allocateItem(name);
    item->mType = kTypeMessage;

    if (obj != NULL) { obj->incStrong(this); }
    item->u.refValue = obj.get();
}

void XMessage::setRect(
        const char *name,
        int32_t left, int32_t top, int32_t right, int32_t bottom) {
    Item *item = allocateItem(name);
    item->mType = kTypeRect;

    item->u.rectValue.mLeft = left;
    item->u.rectValue.mTop = top;
    item->u.rectValue.mRight = right;
    item->u.rectValue.mBottom = bottom;
}

bool XMessage::findStdVector(const char *name, std::vector<int32_t> *vec) const {
    const Item *item = findItem(name, kTypeStdVector);
    if (item) {
        const int size = item->u.int32ArrayValue[0];
        for (int i = 0; i < size; i++) {
            vec->push_back(item->u.int32ArrayValue[i + 1]);
        }
        return true;
    }
    return false;
}

bool XMessage::findString(const char *name, AString *value) const {
    const Item *item = findItem(name, kTypeString);
    if (item) {
        *value = *item->u.stringValue;
        return true;
    }
    return false;
}

bool XMessage::findString8(const char *name, String8 *value) const {
    const Item *item = findItem(name, kTypeString);
    if (item) {
        value->setTo(item->u.stringValue->c_str());
        return true;
    }
    return false;
}

bool XMessage::findObject(const char *name, sp<RefBase> *obj) const {
    const Item *item = findItem(name, kTypeObject);
    if (item) {
        *obj = item->u.refValue;
        return true;
    }
    return false;
}

bool XMessage::findMessage(const char *name, sp<XMessage> *obj) const {
    const Item *item = findItem(name, kTypeMessage);
    if (item) {
        *obj = static_cast<XMessage *>(item->u.refValue);
        return true;
    }
    return false;
}

bool XMessage::findRect(
        const char *name,
        int32_t *left, int32_t *top, int32_t *right, int32_t *bottom) const {
    const Item *item = findItem(name, kTypeRect);
    if (item == NULL) {
        return false;
    }

    *left = item->u.rectValue.mLeft;
    *top = item->u.rectValue.mTop;
    *right = item->u.rectValue.mRight;
    *bottom = item->u.rectValue.mBottom;

    return true;
}

// void XMessage::post(int64_t delayUs) {
//     gLooperRoster.postMessage(this, delayUs);
// }

// status_t XMessage::postAndAwaitResponse(sp<XMessage> *response) {
//     return gLooperRoster.postAndAwaitResponse(this, response);
// }

// void XMessage::postReply(uint32_t replyID) {
//     gLooperRoster.postReply(replyID, this);
// }

bool XMessage::senderAwaitsResponse(uint32_t *replyID) const {
    int32_t tmp;
    bool found = findInt32("replyID", &tmp);

    if (!found) {
        return false;
    }

    *replyID = static_cast<uint32_t>(tmp);

    return true;
}

sp<XMessage> XMessage::dup() const {
    sp<XMessage> msg = new XMessage(mWhat);
    msg->mNumItems = mNumItems;

    for (size_t i = 0; i < mNumItems; ++i) {
        const Item *from = &mItems[i];
        Item *to = &msg->mItems[i];

        to->mName = from->mName;
        to->mType = from->mType;

        switch (from->mType) {
            case kTypeString:
            {
                to->u.stringValue =
                    new AString(*from->u.stringValue);
                break;
            }

            case kTypeObject:
            {
                to->u.refValue = from->u.refValue;
                to->u.refValue->incStrong(msg.get());
                break;
            }

            case kTypeMessage:
            {
                sp<XMessage> copy =
                    static_cast<XMessage *>(from->u.refValue)->dup();

                to->u.refValue = copy.get();
                to->u.refValue->incStrong(msg.get());
                break;
            }

            default:
            {
                to->u = from->u;
                break;
            }
        }
    }

    return msg;
}

static void appendIndent(AString *s, int32_t indent) {
    static const char kWhitespace[] =
        "                                        "
        "                                        ";

    CHECK_LT((size_t)indent, sizeof(kWhitespace));

    s->append(kWhitespace, indent);
}

static bool isFourcc(uint32_t what) {
    return isprint(what & 0xff)
        && isprint((what >> 8) & 0xff)
        && isprint((what >> 16) & 0xff)
        && isprint((what >> 24) & 0xff);
}

AString XMessage::debugString(int32_t indent) const {
    AString s = "XMessage(what = ";

    AString tmp;
    if (isFourcc(mWhat)) {
        tmp = StringPrintf(
                "'%c%c%c%c'",
                (char)(mWhat >> 24),
                (char)((mWhat >> 16) & 0xff),
                (char)((mWhat >> 8) & 0xff),
                (char)(mWhat & 0xff));
    } else {
        tmp = StringPrintf("0x%08x", mWhat);
    }
    s.append(tmp);
    s.append(") = {\n");

    for (size_t i = 0; i < mNumItems; ++i) {
        const Item &item = mItems[i];

        switch (item.mType) {
            case kTypeInt32:
                tmp = StringPrintf(
                        "int32_t %s = %d", item.mName, item.u.int32Value);
                break;
            case kTypeInt64:
                tmp = StringPrintf(
                        "int64_t %s = %lld", item.mName, item.u.int64Value);
                break;
            case kTypeSize:
                tmp = StringPrintf(
                        "size_t %s = %d", item.mName, item.u.sizeValue);
                break;
            case kTypeFloat:
                tmp = StringPrintf(
                        "float %s = %f", item.mName, item.u.floatValue);
                break;
            case kTypeDouble:
                tmp = StringPrintf(
                        "double %s = %f", item.mName, item.u.doubleValue);
                break;
            case kTypePointer:
                tmp = StringPrintf(
                        "void *%s = %p", item.mName, item.u.ptrValue);
                break;
            case kTypeString:
                tmp = StringPrintf(
                        "string %s = \"%s\"",
                        item.mName,
                        item.u.stringValue->c_str());
                break;
            case kTypeObject:
                tmp = StringPrintf(
                        "RefBase *%s = %p", item.mName, item.u.refValue);
                break;
            case kTypeMessage:
                tmp = StringPrintf(
                        "XMessage %s = %s",
                        item.mName,
                        static_cast<XMessage *>(
                            item.u.refValue)->debugString(
                                indent + strlen(item.mName) + 14).c_str());
                break;
            case kTypeRect:
                tmp = StringPrintf(
                        "Rect %s(%d, %d, %d, %d)",
                        item.mName,
                        item.u.rectValue.mLeft,
                        item.u.rectValue.mTop,
                        item.u.rectValue.mRight,
                        item.u.rectValue.mBottom);
                break;
            default:
                TRESPASS();
        }

        appendIndent(&s, indent);
        s.append("  ");
        s.append(tmp);
        s.append("\n");
    }

    appendIndent(&s, indent);
    s.append("}");

    return s;
}

// static
sp<XMessage> XMessage::fromParcel(const Parcel &parcel) {
    int32_t what = parcel.readInt32();
    sp<XMessage> msg = new XMessage(what);

    msg->mNumItems = static_cast<size_t>(parcel.readInt32());

    for (size_t i = 0; i < msg->mNumItems; ++i) {
        Item *item = &msg->mItems[i];

        item->mName = AAtomizer::Atomize(parcel.readCString());
        item->mType = static_cast<Type>(parcel.readInt32());

        switch (item->mType) {
            case kTypeInt32:
            {
                item->u.int32Value = parcel.readInt32();
                break;
            }

            case kTypeInt64:
            {
                item->u.int64Value = parcel.readInt64();
                break;
            }

            case kTypeSize:
            {
                item->u.sizeValue = static_cast<size_t>(parcel.readInt32());
                break;
            }

            case kTypeFloat:
            {
                item->u.floatValue = parcel.readFloat();
                break;
            }

            case kTypeDouble:
            {
                item->u.doubleValue = parcel.readDouble();
                break;
            }

            case kTypeString:
            {
                item->u.stringValue = new AString(parcel.readCString());
                break;
            }

            case kTypeStdVector:
            {
                const int sz = parcel.readInt32();
                item->u.int32ArrayValue = new int32_t[sz + 1];
                item->u.int32ArrayValue[0] = sz;
                for (int i = 0; i < sz; i++) {
                    item->u.int32ArrayValue[i + 1] = parcel.readInt32();
                }
                break;
            }

            case kTypeMessage:
            {
                sp<XMessage> subMsg = XMessage::fromParcel(parcel);
                subMsg->incStrong(msg.get());

                item->u.refValue = subMsg.get();
                break;
            }

            default:
            {
                LOGE("This type of object cannot cross process boundaries.");
                TRESPASS();
            }
        }
    }

    return msg;
}

void XMessage::writeToParcel(Parcel *parcel) const {
    parcel->writeInt32(static_cast<int32_t>(mWhat));
    parcel->writeInt32(static_cast<int32_t>(mNumItems));

    for (size_t i = 0; i < mNumItems; ++i) {
        const Item &item = mItems[i];

        parcel->writeCString(item.mName);
        parcel->writeInt32(static_cast<int32_t>(item.mType));

        switch (item.mType) {
            case kTypeInt32:
            {
                parcel->writeInt32(item.u.int32Value);
                break;
            }

            case kTypeInt64:
            {
                parcel->writeInt64(item.u.int64Value);
                break;
            }

            case kTypeSize:
            {
                parcel->writeInt32(static_cast<int32_t>(item.u.sizeValue));
                break;
            }

            case kTypeFloat:
            {
                parcel->writeFloat(item.u.floatValue);
                break;
            }

            case kTypeDouble:
            {
                parcel->writeDouble(item.u.doubleValue);
                break;
            }

            case kTypeString:
            {
                parcel->writeCString(item.u.stringValue->c_str());
                break;
            }

            case kTypeMessage:
            {
                static_cast<XMessage *>(item.u.refValue)->writeToParcel(parcel);
                break;
            }

            case kTypeStdVector:
            {
                const int sz = item.u.int32ArrayValue[0];
                parcel->writeInt32(sz);
                for (int i = 0; i < sz; i++) {
                    parcel->writeInt32(item.u.int32ArrayValue[i+1]);
                }
                break;
            }

            default:
            {
                LOGE("This type of object cannot cross process boundaries.");
                TRESPASS();
            }
        }
    }
}

}  // namespace android
