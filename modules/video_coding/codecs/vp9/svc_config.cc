/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_coding/codecs/vp9/svc_config.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "modules/video_coding/include/video_codec_interface.h"

namespace webrtc {

namespace {
const int kMinVp9SvcBitrateKbps = 30;  // Lowest VP9 video rate in kbps.
}  // namespace

std::vector<SpatialLayer> GetSvcConfig(size_t input_width,
                                       size_t input_height,
                                       size_t num_spatial_layers,
                                       size_t num_temporal_layers) {
  RTC_DCHECK_GT(input_width, 0);
  RTC_DCHECK_GT(input_height, 0);
  RTC_DCHECK_GT(num_spatial_layers, 0);
  RTC_DCHECK_GT(num_temporal_layers, 0);

  std::vector<SpatialLayer> spatial_layers;

  // Limit number of layers for given resolution.
  const size_t num_layers_fit_horz = static_cast<size_t>(std::floor(
      1 + std::max(0.0f,
                   std::log2(1.0f * input_width / kMinVp9SpatialLayerWidth))));
  const size_t num_layers_fit_vert = static_cast<size_t>(
      std::floor(1 + std::max(0.0f, std::log2(1.0f * input_height /
                                              kMinVp9SpatialLayerHeight))));
  num_spatial_layers =
      std::min({num_spatial_layers, num_layers_fit_horz, num_layers_fit_vert});

  for (size_t sl_idx = 0; sl_idx < num_spatial_layers; ++sl_idx) {
    SpatialLayer spatial_layer = {0};
    spatial_layer.width = input_width >> (num_spatial_layers - sl_idx - 1);
    spatial_layer.height = input_height >> (num_spatial_layers - sl_idx - 1);
    spatial_layer.numberOfTemporalLayers = num_temporal_layers;

    // minBitrate and maxBitrate formulas were derived from
    // subjective-quality data to determing bit rates below which video
    // quality is unacceptable and above which additional bits do not provide
    // benefit. The formulas express rate in units of kbps.

    // TODO(ssilkin): Add to the comment PSNR/SSIM we get at encoding certain
    // video to min/max bitrate specified by those formulas.
    const size_t num_pixels = spatial_layer.width * spatial_layer.height;
    const int min_bitrate =
        static_cast<int>((600. * std::sqrt(num_pixels) - 95000.) / 1000.);
    spatial_layer.minBitrate = std::max(min_bitrate, kMinVp9SvcBitrateKbps);
    spatial_layer.maxBitrate =
         static_cast<int>((1.6 * num_pixels + 50 * 1000) / 1000);
    spatial_layer.targetBitrate =
        (spatial_layer.maxBitrate + spatial_layer.minBitrate) / 2;

    spatial_layers.push_back(spatial_layer);
  }

  return spatial_layers;
}

}  // namespace webrtc
