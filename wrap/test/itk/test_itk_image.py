"""
Copyright (C) 2020 Pablo Hernande-Cerdan.

This file is part of SGEXT: http://github.com/phcerdan/sgext.

This file may be used under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation and appearing in
the file LICENSE.GPL included in the packaging of this file.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""

from sgext import itk
from sgext import scripts
from sgext import tables_folder
import unittest
import os, tempfile
import numpy as np

class TestITKImagePointer(unittest.TestCase):
    # This setup only occurs once for the the whole class
    @classmethod
    def setUpClass(cls):
        # Create temp directory
        cls.test_dir = tempfile.mkdtemp()
        dirname = os.path.dirname(os.path.abspath(__file__))
        cls.input = os.path.join(dirname, '../../../images/bX3D.tif')
        if not os.path.exists(cls.input):
            raise "Input image for thin script in test_itk_image not found: " + cls.input
        cls.img =scripts.thin_io(input_file=cls.input,
                          out_folder=cls.test_dir,
                          foreground="black",
                          skel_type="end", select_type="first",
                          tables_folder=tables_folder,
                          persistence=2,
                          visualize=False,
                          verbose=False)


    def test_basics(self):
        # Default Constructor (empty)
        image = itk.IUC3P()
        # Read-only functions
        self.assertEqual(image.dimension(), 3)
        self.assertAlmostEqual(image.size().all(), 0.)
        self.assertAlmostEqual(image.origin().all(), 0.)
        self.assertAlmostEqual(image.spacing().all(), 1.)
        self.assertAlmostEqual(image.direction()[0,0], 1.)

    def test_set_direction(self):
        image = itk.IUC3P()
        np_dir = np.array([[1, 1, 0], [0, 1, 1], [0, 0, 1]])
        image.set_direction(np_dir)
        bad_dir = np.array([[1, 1, 0], [0, 1, 1]])
        self.assertRaises(RuntimeError, image.set_direction, bad_dir)
        image.set_direction(np.eye(3))

    def test_set_origin(self):
        image = itk.IUC3P()
        np_origin = np.array([1, 2, 0])
        image.set_origin(np_origin)
        bad_origin = np.array([1, 1])
        self.assertRaises(RuntimeError, image.set_origin, bad_origin)

    def test_set_spacing(self):
        image = itk.IUC3P()
        np_spacing = np.array([1, 2, 1])
        image.set_spacing(np_spacing)
        bad_spacing = np.array([1, 1])
        self.assertRaises(RuntimeError, image.set_spacing, bad_spacing)

    def test_set_region(self):
        image = itk.IUC3P()
        np_index = np.array([1, 1, 1])
        np_size = np.array([5, 6, 7])
        image.set_region(index=np_index, size=np_size)
        negative_size = np.array([-5, 6, 7])
        self.assertRaises(RuntimeError, image.set_region, np_index, negative_size)

    def test_set_size(self):
        image = itk.IUC3P()
        np_size = np.array([5, 6, 7])
        image.set_size(np_size)
        negative_size = np.array([-5, 6, 7])
        self.assertRaises(RuntimeError, image.set_size, negative_size)

    def test_get_pixel(self):
        image = self.img
        np_index = np.array([1, 1, 1])
        value_array = image.get_pixel([1,1,1])
        value_np = image.get_pixel(np_index)
        self.assertEqual(value_array, 0)
        self.assertEqual(value_array, value_np)

    def test_set_pixel(self):
        image = self.img
        index = [1,1,1]
        value_array = image.get_pixel(index)
        self.assertEqual(value_array, 0)
        image.set_pixel(index, 23)
        self.assertEqual(image.get_pixel(index), 23)

    def test_physical_point_to_index(self):
        image = self.img
        physical_point = [1, 1, 1]
        index = image.transform_physical_point_to_index(physical_point)
        is_inside = image.is_inside(index)
        self.assertTrue(is_inside)
        np.testing.assert_array_equal(index, [3,3,1])

    def test_physical_point_to_continuous_index(self):
        image = self.img
        physical_point = [1, 1, 1]
        continuous_index = image.transform_physical_point_to_continuous_index(physical_point)
        is_inside = image.is_inside(continuous_index)
        self.assertTrue(is_inside)
        np.testing.assert_array_almost_equal(continuous_index, [2.835, 2.835, 1.])

    def test_index_to_physical_point(self):
        image = self.img
        index = [3,3,1]
        physical_point = image.transform_index_to_physical_point(index)
        np.testing.assert_array_almost_equal(physical_point, [1.05820104, 1.05820104, 1.])

    def test_to_pyarray(self):
        img = self.img
        self.assertEqual(img.dimension(), 3)
        np.testing.assert_array_equal(img.size(), [50,50,7])
        self.assertAlmostEqual(img.origin().all(), 0.)
        np.testing.assert_array_almost_equal(img.spacing(), [0.352734, 0.352734, 1])
        self.assertAlmostEqual(img.direction()[0,0], 1.)

        # Fortran contiguous (Default)
        arr_f = img.to_pyarray(contig="F")
        np.testing.assert_array_equal(arr_f.shape, [7, 50, 50])
        # C contiguous
        arr_c = img.to_pyarray(contig="C")
        np.testing.assert_array_equal(arr_c.shape, img.size())

    def test_from_pyarray(self):
        img = self.img
        arr_f = img.to_pyarray(contig="F")

        new_img = itk.IUC3P()
        new_img.from_pyarray(arr_f, contig="F")

        new_arr_f = new_img.to_pyarray(contig="F")
        # Fortran contiguous (Default)
        np.testing.assert_array_equal(new_arr_f.shape, [7, 50, 50])
        # C contiguous
        new_arr_c = new_img.to_pyarray(contig="C")
        np.testing.assert_array_equal(new_arr_c.shape, img.size())

        arr_c = img.to_pyarray(contig="C")
        new_img.from_pyarray(arr_c, contig="C")
        new_arr_f = new_img.to_pyarray(contig="F")
        # Fortran contiguous (Default)
        np.testing.assert_array_equal(new_arr_f.shape, [7, 50, 50])
        # C contiguous
        new_arr_c = new_img.to_pyarray(contig="C")
        np.testing.assert_array_equal(new_arr_c.shape, img.size())

    def test_export_to_itk(self):
        try:
            import itk
        except:
            print("Warning: itk python package not found. Not testing exporting to itk")
            return

        img = self.img
        # Test with C contiguous
        arr_c = img.to_pyarray(contig='C')
        itk_img_c = itk.GetImageFromArray(arr_c)
        self.assertEqual(arr_c.shape[0], 50)
        subs = itk.subtract_image_filter(itk_img_c, itk_img_c)
        subs_np = itk.GetArrayViewFromImage(subs) # no-copy
        self.assertEqual(subs_np.max(), 0)

        # Test with Fortran contiguous
        # ITK always assume than input numpy array will be F contig (which is easier!)
        arr_f = img.to_pyarray(contig='F')
        itk_img_f = itk.GetImageFromArray(arr_f)
        self.assertEqual(arr_f.shape[0], 7)
        self.assertEqual(itk_img_f.GetLargestPossibleRegion().GetSize()[0], 50)

        # Check usability with a filter
        subs = itk.subtract_image_filter(itk_img_f, itk_img_f)
        subs_np = itk.GetArrayViewFromImage(subs) # no-copy
        self.assertEqual(subs_np.max(), 0)





