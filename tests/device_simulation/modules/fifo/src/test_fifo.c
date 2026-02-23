// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>
#include <stdint.h>

#include "fifo.h"

uint8_t fifo_buffer[2];
struct fifo test_fifo;

void setUp(){}
void tearDown(){}

void test_fifo_init_reports_OK(void) {
    int status = fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
}

void test_fifo_init_reports_empty(void) {
    int status = fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    TEST_ASSERT_TRUE(fifo_is_empty(&test_fifo));
}

void test_fifo_init_reports_size_zero(void) {
    int status = fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    TEST_ASSERT_EQUAL(0, fifo_size(&test_fifo));
}

void test_fifo_init_with_null_buffer_reports_error(void) {
    int status = fifo_init(&test_fifo, NULL, sizeof(fifo_buffer));
    TEST_ASSERT_EQUAL(FIFO_BAD_PARAM, status);
}

void test_fifo_init_with_null_fifo_reports_error(void) {
    int status = fifo_init(NULL, fifo_buffer, sizeof(fifo_buffer));
    TEST_ASSERT_EQUAL(FIFO_BAD_PARAM, status);
}

void test_fifo_enqueue_reports_OK(void) {
    fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    int status = fifo_enqueue(&test_fifo, 0x55);
    TEST_ASSERT_EQUAL(FIFO_OK, status);
}

void test_fifo_enqueue_reports_not_empty(void) {
    fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    int status = fifo_enqueue(&test_fifo, 0x55);
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    TEST_ASSERT_FALSE(fifo_is_empty(&test_fifo));
}

void test_fifo_enqueue_twice_reports_not_full(void) {
    fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    int status = fifo_enqueue(&test_fifo, 0x55);
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    status = fifo_enqueue(&test_fifo, 0xAA);
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    TEST_ASSERT_TRUE(fifo_is_full(&test_fifo));
}

void test_fifo_enqueue_when_full_reports_full(void) {
    fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    fifo_enqueue(&test_fifo, 0x55);
    fifo_enqueue(&test_fifo, 0xAA);
    int status = fifo_enqueue(&test_fifo, 0xFF);
    TEST_ASSERT_EQUAL(FIFO_FULL, status);
}

void test_fifo_dequeue_reports_OK_and_correct_data(void) {
    fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    fifo_enqueue(&test_fifo, 0x55);
    struct fifo_status_data result = fifo_dequeue(&test_fifo);
    TEST_ASSERT_EQUAL(FIFO_OK, result.status);
    TEST_ASSERT_EQUAL(0x55, result.data);
}

void test_fifo_dequeue_when_empty_reports_empty(void) {
    fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    struct fifo_status_data result = fifo_dequeue(&test_fifo);
    TEST_ASSERT_EQUAL(FIFO_EMPTY, result.status);
}

void test_fifo_size_reports_correct_size(void) {
    fifo_init(&test_fifo, fifo_buffer, sizeof(fifo_buffer));
    fifo_enqueue(&test_fifo, 0x55);
    fifo_enqueue(&test_fifo, 0xAA);
    TEST_ASSERT_EQUAL(2, fifo_size(&test_fifo));
}

/* Block operations */
uint8_t block_fifo_buffer[4];
struct fifo block_test_fifo;

void test_fifo_block_enqueue_reports_OK(void) {
    fifo_init(&block_test_fifo, block_fifo_buffer, sizeof(block_fifo_buffer));
    uint8_t data_to_enqueue[2] = {0x11, 0x22};
    int status = fifo_block_enqueue(&block_test_fifo, data_to_enqueue, sizeof(data_to_enqueue));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
}

void test_fifo_block_enqueue_overfill_reports_full(void) {
    fifo_init(&block_test_fifo, block_fifo_buffer, sizeof(block_fifo_buffer));
    uint8_t data_to_enqueue[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    int status = fifo_block_enqueue(&block_test_fifo, data_to_enqueue, sizeof(data_to_enqueue));
    TEST_ASSERT_EQUAL(FIFO_FULL, status);
}

void test_fifo_block_enqueue_with_null_fifo_reports_bad_param(void) {
    uint8_t data_to_enqueue[2] = {0x11, 0x22};
    int status = fifo_block_enqueue(NULL, data_to_enqueue, sizeof(data_to_enqueue));
    TEST_ASSERT_EQUAL(FIFO_BAD_PARAM, status);
}

void test_fifo_block_enqueue_readback_with_correct_data(void) {
    fifo_init(&block_test_fifo, block_fifo_buffer, sizeof(block_fifo_buffer));
    uint8_t data_to_enqueue[2] = {0x11, 0x22};
    int status = fifo_block_enqueue(&block_test_fifo, data_to_enqueue, sizeof(data_to_enqueue));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    /* dequeue character at a time */
    uint8_t data_dequeued[2] = {0};
    struct fifo_status_data result1 = fifo_dequeue(&block_test_fifo);
    TEST_ASSERT_EQUAL(FIFO_OK, result1.status);
    data_dequeued[0] = result1.data;
    struct fifo_status_data result2 = fifo_dequeue(&block_test_fifo);
    TEST_ASSERT_EQUAL(FIFO_OK, result2.status);
    data_dequeued[1] = result2.data;
    TEST_ASSERT_EQUAL(0x11, data_dequeued[0]);
    TEST_ASSERT_EQUAL(0x22, data_dequeued[1]);
}

void test_fifo_block_enqueue_wrap_reports_OK_and_correct_size(void) {
    fifo_init(&block_test_fifo, block_fifo_buffer, sizeof(block_fifo_buffer));
    /* force move of indices to force wrap in following operations */
    block_test_fifo.head = 2;
    block_test_fifo.tail = 2;
    /* Fill to capacity to force wrap */
    uint8_t data_to_enqueue1[4] = {0x11, 0x22, 0x33, 0x44};
    int status = fifo_block_enqueue(&block_test_fifo, data_to_enqueue1, sizeof(data_to_enqueue1));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    TEST_ASSERT_EQUAL(sizeof(data_to_enqueue1), fifo_size(&block_test_fifo));
}

void test_fifo_block_enqueue_wrap_reports_correct_data(void) {
    fifo_init(&block_test_fifo, block_fifo_buffer, sizeof(block_fifo_buffer));
    /* force move of indices to force wrap in following operations */
    block_test_fifo.head = 2;
    block_test_fifo.tail = 2;
    /* Fill to capacity to force wrap */
    uint8_t data_to_enqueue1[4] = {0x11, 0x22, 0x33, 0x44};
    int status = fifo_block_enqueue(&block_test_fifo, data_to_enqueue1, sizeof(data_to_enqueue1));
    TEST_ASSERT_EQUAL(FIFO_OK, status);

    /* dequeue character at a time */
    uint8_t data_dequeued[4] = {0};
    for (size_t i = 0; i < sizeof(data_dequeued); i++) {
        struct fifo_status_data result = fifo_dequeue(&block_test_fifo);
        TEST_ASSERT_EQUAL(FIFO_OK, result.status);
        data_dequeued[i] = result.data;
    }
    TEST_ASSERT_EQUAL(0x11, data_dequeued[0]);
    TEST_ASSERT_EQUAL(0x22, data_dequeued[1]);
    TEST_ASSERT_EQUAL(0x33, data_dequeued[2]);
    TEST_ASSERT_EQUAL(0x44, data_dequeued[3]);
}

void test_fifo_block_dequeue_reports_OK_and_correct_data(void) {
    fifo_init(&block_test_fifo, block_fifo_buffer, sizeof(block_fifo_buffer));
    uint8_t data_to_enqueue[2] = {0x11, 0x22};
    fifo_block_enqueue(&block_test_fifo, data_to_enqueue, sizeof(data_to_enqueue));

    uint8_t data_dequeued[2] = {0};
    int status = fifo_block_dequeue(&block_test_fifo, data_dequeued, sizeof(data_dequeued));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    TEST_ASSERT_EQUAL(0x11, data_dequeued[0]);
    TEST_ASSERT_EQUAL(0x22, data_dequeued[1]);
}

void test_fifo_block_enqueue_wrap_dequeue_correct_data(void) {
    fifo_init(&block_test_fifo, block_fifo_buffer, sizeof(block_fifo_buffer));
    /* force move of indices to force wrap in following operations */
    block_test_fifo.head = 2;
    block_test_fifo.tail = 2;
    /* Fill to capacity to force wrap */
    uint8_t data_to_enqueue1[4] = {0x11, 0x22, 0x33, 0x44};
    int status = fifo_block_enqueue(&block_test_fifo, data_to_enqueue1, sizeof(data_to_enqueue1));
    TEST_ASSERT_EQUAL(FIFO_OK, status);

    uint8_t data_dequeued[4] = {0};
    status = fifo_block_dequeue(&block_test_fifo, data_dequeued, sizeof(data_dequeued));
    TEST_ASSERT_EQUAL(FIFO_OK, status);
    TEST_ASSERT_EQUAL(0x11, data_dequeued[0]);
    TEST_ASSERT_EQUAL(0x22, data_dequeued[1]);
    TEST_ASSERT_EQUAL(0x33, data_dequeued[2]);
    TEST_ASSERT_EQUAL(0x44, data_dequeued[3]);
}
