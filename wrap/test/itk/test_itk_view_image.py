# Copyright (C) 2019 Pablo Hernandez-Cerdan
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from sgext import itk
from sgext import scripts
import unittest
import os, tempfile

class TestITKViewImage(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Create temp directory
        cls.test_dir = tempfile.mkdtemp()
        dirname = os.path.dirname(os.path.abspath(__file__))
        cls.input = os.path.join(dirname, '../../../images/bX3D.tif')
        if not os.path.exists(cls.input):
            raise "Input image for script used in test_view_image not found: " + cls.input

    # TODO closing is not automated
    def test_from_file(self):
        itk.view_image(input_file=self.input, win_title = "Original image (foreground=black)")

    def test_from_sgext_itk_img(self):
        dmap =scripts.create_distance_map_io(input_file=self.input,
                          out_folder=self.test_dir,
                          foreground="black",
                          use_itk=False,
                          verbose=True)
        itk.view_image(input=dmap, win_title = "DMap")
