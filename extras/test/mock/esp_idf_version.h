#pragma once
#define ESP_IDF_VERSION_VAL(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))
#ifndef MOCK_IDF_MAJOR
#define MOCK_IDF_MAJOR 5
#define MOCK_IDF_MINOR 3
#endif
#define ESP_IDF_VERSION ESP_IDF_VERSION_VAL(MOCK_IDF_MAJOR, MOCK_IDF_MINOR, 0)
