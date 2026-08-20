/*
 * Copyright (C) 2015 Meltytech, LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with consumer library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <mlt++/Mlt.h>
#include <QtTest>
using namespace Mlt;

class TestFilter : public QObject
{
    Q_OBJECT
    mlt_locale_t locale;

public:
    TestFilter()
    {
#if defined(__linux__) || defined(__APPLE__)
        locale = newlocale(LC_NUMERIC_MASK, "POSIX", NULL);
#endif
        Factory::init();
    }

    ~TestFilter()
    {
#if defined(__linux__) || defined(__APPLE__)
        freelocale(locale);
#endif
    }

private Q_SLOTS:
    void ProcessModifiesFrame()
    {
        Profile profile("dv_ntsc");
        Producer producer(profile, "noise", NULL);
        Filter filter(profile, "resize");
        int width = 0;
        int height = 0;
        mlt_image_format format = mlt_image_rgb;

        // Get a frame from the producer
        Frame *frame = producer.get_frame();

        // Get the default image size: width should match profile
        frame->get_image(format, width, height, 0);
        QCOMPARE(width, 720);

        // Without applying the filter, the width request is not honored.
        width = 360;
        frame->get_image(format, width, height, 0);
        QCOMPARE(width, 720);

        // Apply the filter and the requested width will be provided
        width = 360;
        filter.process(*frame);
        frame->get_image(format, width, height, 0);
        QCOMPARE(width, 360);

        delete frame;
    }

    void QtBlendCoverageEdgeCases()
    {
        Profile profile("hdv_1080_25p");
        Producer producer(profile, "color", "red");
        Filter filter(profile, "qtblend");
        filter.set("background_color", 0x12345678);

        // Test microscopic rotation angle within fuzzy threshold (1e-13)
        filter.set("rotation", 1e-13);
        Frame *frame1 = producer.get_frame();
        QVERIFY(frame1 != NULL);
        filter.process(*frame1);
        int width = 1920;
        int height = 1080;
        mlt_image_format format = mlt_image_rgba;
        uint8_t *img1 = frame1->get_image(format, width, height, 0);
        QVERIFY(img1 != NULL);
        uint32_t *pixel1 = reinterpret_cast<uint32_t *>(img1);
        QVERIFY(pixel1 != NULL);
        QVERIFY(pixel1[0] != 0);
        delete frame1;

        // Test microscopic non-1.0 opacity within fuzzy threshold (1.0 - 1e-13)
        filter.set("rotation", 0.0);
        filter.set("rect", "0 0 100% 100% 0.9999999999999");
        Frame *frame2 = producer.get_frame();
        QVERIFY(frame2 != NULL);
        filter.process(*frame2);
        width = 1920;
        height = 1080;
        format = mlt_image_rgba;
        uint8_t *img2 = frame2->get_image(format, width, height, 0);
        QVERIFY(img2 != NULL);
        uint32_t *pixel2 = reinterpret_cast<uint32_t *>(img2);
        QVERIFY(pixel2 != NULL);
        QVERIFY(pixel2[0] != 0);
        delete frame2;

        // Test microscopic rect offset x/y within fuzzy threshold (1e-13)
        filter.set("rect", "0.0000000000001 0.0000000000001 100% 100% 1.0");
        Frame *frame3 = producer.get_frame();
        QVERIFY(frame3 != NULL);
        filter.process(*frame3);
        width = 1920;
        height = 1080;
        format = mlt_image_rgba;
        uint8_t *img3 = frame3->get_image(format, width, height, 0);
        QVERIFY(img3 != NULL);
        uint32_t *pixel3 = reinterpret_cast<uint32_t *>(img3);
        QVERIFY(pixel3 != NULL);
        QVERIFY(pixel3[0] != 0);
        delete frame3;
    }
};

QTEST_APPLESS_MAIN(TestFilter)

#include "test_filter.moc"
