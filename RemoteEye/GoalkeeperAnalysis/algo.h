#ifndef ALGO_H_
#define ALGO_H_

/*

Ausführen:

//Am Anfang zum initialisieren (nur einmal wenn das Programm gestartet wird)
ELSE::m_load("G:\\BENNO Pupil\\base_dat.txt");



//Für jedes Bild
cv::RotatedRect pos=ELSE::run_fast(image);


//Am Ende zum aufräumen (nur einmal wenn das Programm beendet wird)
ELSE::m_clear();

*/

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>


namespace ELSE
{
	int GL_SZ_R_1;
	int GL_SZ_O_1;
	int GL_SZ_D_1;

	float** idx_ppx_1;
	float** idx_ppy_1;
	float*** idx_px_1;
	float*** idx_py_1;
	float*** idx_nx_1;
	float*** idx_ny_1;


	bool** idx_used_1;
	float** idx_weight_1;


	int GL_SZ_R_2;
	int GL_SZ_O_2;
	int GL_SZ_D_2;

	float** idx_ppx_2;
	float** idx_ppy_2;
	float*** idx_px_2;
	float*** idx_py_2;
	float*** idx_nx_2;
	float*** idx_ny_2;


	bool** idx_used_2;
	float** idx_weight_2;

	cv::Mat CNM1_2;
	cv::Mat CNM2_2;

	cv::Mat CNM1_1;
	cv::Mat CNM2_1;

	std::vector<std::vector<cv::Point2i>> INDEXES_P_1;
	std::vector<std::vector<cv::Point2i>> INDEXES_N_1;
	std::vector<std::vector<cv::Point2i>> INDEXES_O_1;

	std::vector<std::vector<cv::Point2i>> INDEXES_P_2;
	std::vector<std::vector<cv::Point2i>> INDEXES_N_2;
	std::vector<std::vector<cv::Point2i>> INDEXES_O_2;

	void m_init_1()
	{
		int sz_r = GL_SZ_R_1;
		int sz_o = GL_SZ_O_1;
		int sz_d = GL_SZ_D_1;


		idx_ppx_1 = new float*[sz_r];
		idx_ppy_1 = new float*[sz_r];

		idx_px_1 = new float**[sz_r];
		idx_py_1 = new float**[sz_r];

		idx_nx_1 = new float**[sz_r];
		idx_ny_1 = new float**[sz_r];

		idx_weight_1 = new float*[sz_r];
		idx_used_1 = new bool*[sz_r];


		for (int r = 0; r < sz_r; r++)
		{
			idx_ppx_1[r] = new float[sz_o];
			idx_ppy_1[r] = new float[sz_o];

			idx_px_1[r] = new float*[sz_o];
			idx_py_1[r] = new float*[sz_o];

			idx_nx_1[r] = new float*[sz_o];
			idx_ny_1[r] = new float*[sz_o];


			idx_weight_1[r] = new float[sz_o];
			idx_used_1[r] = new bool[sz_o];
		}


		for (int r = 0; r < sz_r; r++)
		{
			for (int ro = 0; ro < sz_o; ro++)
			{
				idx_px_1[r][ro] = new float[sz_d];
				idx_py_1[r][ro] = new float[sz_d];

				idx_nx_1[r][ro] = new float[sz_d];
				idx_ny_1[r][ro] = new float[sz_d];
			}
		}
	}

	void m_load_1(std::string path)
	{
		std::ifstream file;
		file.open(path);

		if (file.is_open())
		{
			std::string line;

			std::getline(file, line);
			GL_SZ_R_1 = atoi(line.c_str());

			std::getline(file, line);
			GL_SZ_O_1 = atoi(line.c_str());

			std::getline(file, line);
			GL_SZ_D_1 = atoi(line.c_str());

			m_init_1();


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_ppx_1[r][ro] = atof(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_ppy_1[r][ro] = atof(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_weight_1[r][ro] = atof(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_used_1[r][ro] = atoi(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					for (int d = 0; d < GL_SZ_D_1; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_px_1[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}

			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					for (int d = 0; d < GL_SZ_D_1; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_py_1[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					for (int d = 0; d < GL_SZ_D_1; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_nx_1[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					for (int d = 0; d < GL_SZ_D_1; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_ny_1[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}
		}
	}

	void init_mats_1()
	{
		CNM1_1 = cv::Mat::zeros(100, 100, CV_32FC1);
		CNM2_1 = cv::Mat::zeros(100, 100, CV_32FC1);
	}

	void m_clear_1()
	{
		if (GL_SZ_R_1 > 0 && GL_SZ_O_1 > 0 && GL_SZ_D_1 > 0)
		{
			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				for (int ro = 0; ro < GL_SZ_O_1; ro++)
				{
					delete[] idx_px_1[r][ro];
					delete[] idx_py_1[r][ro];

					delete[] idx_nx_1[r][ro];
					delete[] idx_ny_1[r][ro];
				}
			}


			for (int r = 0; r < GL_SZ_R_1; r++)
			{
				delete[] idx_ppx_1[r];
				delete[] idx_ppy_1[r];

				delete[] idx_px_1[r];
				delete[] idx_py_1[r];

				delete[] idx_nx_1[r];
				delete[] idx_ny_1[r];

				delete[] idx_weight_1[r];
				delete[] idx_used_1[r];
			}


			delete[] idx_ppx_1;
			delete[] idx_ppy_1;

			delete[] idx_px_1;
			delete[] idx_py_1;

			delete[] idx_nx_1;
			delete[] idx_ny_1;

			delete[] idx_weight_1;
			delete[] idx_used_1;
		}
	}

	void cmp_indexes_1()
	{
		INDEXES_P_1.clear();
		INDEXES_N_1.clear();
		INDEXES_O_1.clear();

		for (int i = 20; i < 100 - 15; i++)
		{
			for (int j = 10; j < 100 - 10; j++)
			{
				float val_r = 0;
				float val_conn = 0;
				float val_rc = 0;

				std::vector<cv::Point2i> ak_vec_idx_o;
				ak_vec_idx_o.clear();

				std::vector<cv::Point2i> ak_vec_idx_p;
				std::vector<cv::Point2i> ak_vec_idx_n;
				ak_vec_idx_p.clear();
				ak_vec_idx_n.clear();


				for (int r = 0; r < GL_SZ_R_1 / 2; r++)
				{
					for (int ro = 0; ro < GL_SZ_O_1; ro++)
					{
						if (idx_used_1[r][ro])
						{
							int ppx = idx_ppx_1[r][ro] + j;
							int ppy = idx_ppy_1[r][ro] + i;


							if (ppx > 0 && ppy > 0 && ppx < 100 && ppy < 100)
							{
								float pos = 0;
								float neg = 0;

								float mea_cnt = 0;

								float pos_o = 0;
								float neg_o = 0;

								for (int k = 0; k < GL_SZ_D_1; k++)
								{
									int px = idx_px_1[r][ro][k] + j;
									int py = idx_py_1[r][ro][k] + i;

									int npx = idx_nx_1[r][ro][k] + j;
									int npy = idx_ny_1[r][ro][k] + i;


									if (px > 0 && py > 0 && px < 100 && py < 100 &&
										npx > 0 && npy > 0 && npx < 100 && npy < 100)
									{
										ak_vec_idx_p.push_back(cv::Point2i(px, py));
										ak_vec_idx_n.push_back(cv::Point2i(npx, npy));
										ak_vec_idx_o.push_back(cv::Point2i(ppx, ppy));
									}
								}
							}
						}
					}
				}

				INDEXES_P_1.push_back(ak_vec_idx_p);
				INDEXES_N_1.push_back(ak_vec_idx_n);
				INDEXES_O_1.push_back(ak_vec_idx_o);
			}
		}
	}

	void m_fast_idx_1(cv::Mat& in)
	{
		cv::Scalar mea = cv::mean(in);

		int idx_run = 0;
		for (int i = 20; i < in.rows - 15; i++)
		{
			for (int j = 10; j < in.cols - 10; j++)
			{
				bool last = false;

				float val_r = 0;
				float val_conn = 0;
				float val_rc = 0;


				for (int idx = 0; idx < INDEXES_O_1[idx_run].size(); idx++)
				{
					if (mea[0] * 0.5 > in.at<float>(INDEXES_O_1[idx_run][idx]))
					{
						//works fine same results


						float pos = in.at<float>(INDEXES_P_1[idx_run][idx]);
						float neg = in.at<float>(INDEXES_N_1[idx_run][idx]);

						if (pos < neg * 0.9 && pos < mea[0] && neg < mea[0] && last)
						{
							//0.9
							val_r += val_conn;
						}

						if (pos < neg * 0.9 && pos < mea[0] && neg < mea[0])
						{
							//0.9
							val_r++;
							val_conn++;
							last = true;
						}
						else
						{
							val_conn = 0;
							last = false;
						}
					}

					val_rc++;
				}


				if (val_rc > 0)
				{
					val_r /= val_rc;
					CNM2_1.at<float>(i, j) = val_r;
				}
				else
				{
					CNM2_1.at<float>(i, j) = 0.0f;
				}


				idx_run++;
			}
		}


		CNM2_1.copyTo(in);
	}

	cv::RotatedRect run_fast_idx_1(cv::Mat input)
	{
		cv::RotatedRect ellipse;
		ellipse.size.height = 5;
		ellipse.size.width = 5;

		ellipse.center.x = -1;
		ellipse.center.y = -1;


		if (input.cols < 1 || input.rows < 1)
			return ellipse;

		if (input.channels() > 1)
		{
			if (input.channels() == 3)
				cv::cvtColor(input, input, CV_BGR2GRAY);
			else
				return ellipse;
		}


		input.copyTo(CNM1_1);
		CNM1_1.convertTo(CNM1_1, CV_32FC1);

		cv::normalize(CNM1_1, CNM1_1, 0, 1, cv::NORM_MINMAX, CV_32FC1);

		m_fast_idx_1(CNM1_1);


		cv::boxFilter(CNM1_1, CNM1_1, -1, cv::Size(7, 7));

		float max_val = 0;
		for (int i = 20; i < CNM1_1.rows - 15; i++)
		{
			for (int j = 10; j < CNM1_1.cols - 10; j++)
			{
				if (CNM1_1.at<float>(i, j) > max_val)
				{
					max_val = CNM1_1.at<float>(i, j);
					ellipse.center.x = j;
					ellipse.center.y = i;
				}
			}
		}


		return ellipse;
	}


	void m_init_2()
	{
		int sz_r = GL_SZ_R_2;
		int sz_o = GL_SZ_O_2;
		int sz_d = GL_SZ_D_2;


		idx_ppx_2 = new float*[sz_r];
		idx_ppy_2 = new float*[sz_r];

		idx_px_2 = new float**[sz_r];
		idx_py_2 = new float**[sz_r];

		idx_nx_2 = new float**[sz_r];
		idx_ny_2 = new float**[sz_r];

		idx_weight_2 = new float*[sz_r];
		idx_used_2 = new bool*[sz_r];


		for (int r = 0; r < sz_r; r++)
		{
			idx_ppx_2[r] = new float[sz_o];
			idx_ppy_2[r] = new float[sz_o];

			idx_px_2[r] = new float*[sz_o];
			idx_py_2[r] = new float*[sz_o];

			idx_nx_2[r] = new float*[sz_o];
			idx_ny_2[r] = new float*[sz_o];


			idx_weight_2[r] = new float[sz_o];
			idx_used_2[r] = new bool[sz_o];
		}


		for (int r = 0; r < sz_r; r++)
		{
			for (int ro = 0; ro < sz_o; ro++)
			{
				idx_px_2[r][ro] = new float[sz_d];
				idx_py_2[r][ro] = new float[sz_d];

				idx_nx_2[r][ro] = new float[sz_d];
				idx_ny_2[r][ro] = new float[sz_d];
			}
		}
	}

	void m_load_2(std::string path)
	{
		std::ifstream file;
		file.open(path);

		if (file.is_open())
		{
			std::string line;

			std::getline(file, line);
			GL_SZ_R_2 = atoi(line.c_str());

			std::getline(file, line);
			GL_SZ_O_2 = atoi(line.c_str());

			std::getline(file, line);
			GL_SZ_D_2 = atoi(line.c_str());

			m_init_2();


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_ppx_2[r][ro] = atof(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_ppy_2[r][ro] = atof(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_weight_2[r][ro] = atof(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					std::string ak_val = "";


					while (line[line_idx] != ';')
					{
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_used_2[r][ro] = atoi(ak_val.c_str());
				}
			}


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					for (int d = 0; d < GL_SZ_D_2; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_px_2[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}

			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					for (int d = 0; d < GL_SZ_D_2; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_py_2[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					for (int d = 0; d < GL_SZ_D_2; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_nx_2[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				std::getline(file, line);
				int line_idx = 0;
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					for (int d = 0; d < GL_SZ_D_2; d++)
					{
						std::string ak_val = "";

						while (line[line_idx] != ';')
						{
							ak_val += line[line_idx];
							line_idx++;
						}
						line_idx++;

						idx_ny_2[r][ro][d] = atof(ak_val.c_str());
					}
				}
			}
		}
	}


	void init_mats_2()
	{
		CNM1_2 = cv::Mat::zeros(100, 100, CV_32FC1);
		CNM2_2 = cv::Mat::zeros(100, 100, CV_32FC1);
	}

	void m_clear_2()
	{
		if (GL_SZ_R_2 > 0 && GL_SZ_O_2 > 0 && GL_SZ_D_2 > 0)
		{
			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				for (int ro = 0; ro < GL_SZ_O_2; ro++)
				{
					delete[] idx_px_2[r][ro];
					delete[] idx_py_2[r][ro];

					delete[] idx_nx_2[r][ro];
					delete[] idx_ny_2[r][ro];
				}
			}


			for (int r = 0; r < GL_SZ_R_2; r++)
			{
				delete[] idx_ppx_2[r];
				delete[] idx_ppy_2[r];

				delete[] idx_px_2[r];
				delete[] idx_py_2[r];

				delete[] idx_nx_2[r];
				delete[] idx_ny_2[r];

				delete[] idx_weight_2[r];
				delete[] idx_used_2[r];
			}


			delete[] idx_ppx_2;
			delete[] idx_ppy_2;

			delete[] idx_px_2;
			delete[] idx_py_2;

			delete[] idx_nx_2;
			delete[] idx_ny_2;

			delete[] idx_weight_2;
			delete[] idx_used_2;
		}
	}


	void cmp_indexes_2()
	{
		INDEXES_P_2.clear();
		INDEXES_N_2.clear();
		INDEXES_O_2.clear();

		for (int i = 20; i < 100 - 15; i++)
		{
			for (int j = 10; j < 100 - 10; j++)
			{
				float val_r = 0;
				float val_conn = 0;
				float val_rc = 0;

				std::vector<cv::Point2i> ak_vec_idx_o;
				ak_vec_idx_o.clear();

				std::vector<cv::Point2i> ak_vec_idx_p;
				std::vector<cv::Point2i> ak_vec_idx_n;
				ak_vec_idx_p.clear();
				ak_vec_idx_n.clear();


				for (int r = 0; r < GL_SZ_R_2 / 2; r++)
				{
					for (int ro = 0; ro < GL_SZ_O_2; ro++)
					{
						if (idx_used_2[r][ro])
						{
							int ppx = idx_ppx_2[r][ro] + j;
							int ppy = idx_ppy_2[r][ro] + i;


							if (ppx > 0 && ppy > 0 && ppx < 100 && ppy < 100)
							{
								float pos = 0;
								float neg = 0;

								float mea_cnt = 0;

								float pos_o = 0;
								float neg_o = 0;

								for (int k = 0; k < GL_SZ_D_2; k++)
								{
									int px = idx_px_2[r][ro][k] + j;
									int py = idx_py_2[r][ro][k] + i;

									int npx = idx_nx_2[r][ro][k] + j;
									int npy = idx_ny_2[r][ro][k] + i;


									if (px > 0 && py > 0 && px < 100 && py < 100 &&
										npx > 0 && npy > 0 && npx < 100 && npy < 100)
									{
										ak_vec_idx_p.push_back(cv::Point2i(px, py));
										ak_vec_idx_n.push_back(cv::Point2i(npx, npy));
										ak_vec_idx_o.push_back(cv::Point2i(ppx, ppy));
									}
								}
							}
						}
					}
				}

				INDEXES_P_2.push_back(ak_vec_idx_p);
				INDEXES_N_2.push_back(ak_vec_idx_n);
				INDEXES_O_2.push_back(ak_vec_idx_o);
			}
		}
	}

	void m_fast_idx_2(cv::Mat& in)
	{
		cv::Scalar mea = cv::mean(in);

		int idx_run = 0;
		for (int i = 20; i < in.rows - 15; i++)
		{
			for (int j = 10; j < in.cols - 10; j++)
			{
				bool last = false;

				float val_r = 0;
				float val_conn = 0;
				float val_rc = 0;


				for (int idx = 0; idx < INDEXES_O_2[idx_run].size(); idx++)
				{
					if (mea[0] * 0.5 > in.at<float>(INDEXES_O_2[idx_run][idx]))
					{
						//works fine same results


						float pos = in.at<float>(INDEXES_P_2[idx_run][idx]);
						float neg = in.at<float>(INDEXES_N_2[idx_run][idx]);

						if (pos < neg * 0.9 && pos < mea[0] && neg < mea[0] && last)
						{
							//0.9
							val_r += val_conn;
						}

						if (pos < neg * 0.9 && pos < mea[0] && neg < mea[0])
						{
							//0.9
							val_r++;
							val_conn++;
							last = true;
						}
						else
						{
							val_conn = 0;
							last = false;
						}
					}

					val_rc++;
				}


				if (val_rc > 0)
				{
					val_r /= val_rc;
					CNM2_2.at<float>(i, j) = val_r;
				}
				else
				{
					CNM2_2.at<float>(i, j) = 0.0f;
				}


				idx_run++;
			}
		}


		CNM2_2.copyTo(in);
	}


	cv::RotatedRect run_fast_idx_2(cv::Mat input)
	{
		cv::RotatedRect ellipse;
		ellipse.size.height = 5;
		ellipse.size.width = 5;

		ellipse.center.x = -1;
		ellipse.center.y = -1;


		if (input.cols < 1 || input.rows < 1)
			return ellipse;

		if (input.channels() > 1)
		{
			if (input.channels() == 3)
				cv::cvtColor(input, input, CV_BGR2GRAY);
			else
				return ellipse;
		}


		input.copyTo(CNM1_2);
		CNM1_2.convertTo(CNM1_2, CV_32FC1);

		cv::normalize(CNM1_2, CNM1_2, 0, 1, cv::NORM_MINMAX, CV_32FC1);

		m_fast_idx_2(CNM1_2);


		cv::boxFilter(CNM1_2, CNM1_2, -1, cv::Size(7, 7));

		float max_val = 0;
		for (int i = 20; i < CNM1_2.rows - 15; i++)
		{
			for (int j = 10; j < CNM1_2.cols - 10; j++)
			{
				if (CNM1_2.at<float>(i, j) > max_val)
				{
					max_val = CNM1_2.at<float>(i, j);
					ellipse.center.x = j;
					ellipse.center.y = i;
				}
			}
		}


		return ellipse;
	}
}


/*
#include <iostream>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>


namespace ELSE{

int GL_SZ_R;
int GL_SZ_O;
int GL_SZ_D;

float **idx_ppx;
float **idx_ppy;
float ***idx_px;
float ***idx_py;
float ***idx_nx;
float ***idx_ny;

bool **idx_used;
float **idx_weight;





static void m_init(){

int sz_r=GL_SZ_R;
int sz_o=GL_SZ_O;
int sz_d=GL_SZ_D;


idx_ppx=new float*[sz_r];
idx_ppy=new float*[sz_r];

idx_px=new float**[sz_r];
idx_py=new float**[sz_r];

idx_nx=new float**[sz_r];
idx_ny=new float**[sz_r];

idx_weight=new float*[sz_r];
idx_used=new bool*[sz_r];


for(int r=0;r<sz_r;r++){
idx_ppx[r]=new float[sz_o];
idx_ppy[r]=new float[sz_o];

idx_px[r]=new float*[sz_o];
idx_py[r]=new float*[sz_o];

idx_nx[r]=new float*[sz_o];
idx_ny[r]=new float*[sz_o];


idx_weight[r]=new float[sz_o];
idx_used[r]=new bool[sz_o];
}


for(int r=0;r<sz_r;r++){
for(int ro=0;ro<sz_o;ro++){

idx_px[r][ro]=new float[sz_d];
idx_py[r][ro]=new float[sz_d];

idx_nx[r][ro]=new float[sz_d];
idx_ny[r][ro]=new float[sz_d];

}
}

}


static void m_store(std::string path){

if(path != "" && GL_SZ_R>0 && GL_SZ_O>0 && GL_SZ_D>0){


std::ofstream file;
file.open (path,std::ios::trunc);
file << GL_SZ_R <<std::endl;
file << GL_SZ_O <<std::endl;
file << GL_SZ_D <<std::endl;


for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

file << idx_ppx[r][ro] << ";";
}
file<<std::endl;
}


for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

file << idx_ppy[r][ro] << ";";
}
file<<std::endl;
}



for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

file << idx_weight[r][ro] << ";";
}
file<<std::endl;
}


for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

file << idx_used[r][ro] << ";";
}
file<<std::endl;
}




for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

for(int d=0;d<GL_SZ_D;d++){
file << idx_px[r][ro][d] << ";";
}
}
file<<std::endl;
}


for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

for(int d=0;d<GL_SZ_D;d++){
file << idx_py[r][ro][d] << ";";
}
}
file<<std::endl;
}


for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

for(int d=0;d<GL_SZ_D;d++){
file << idx_nx[r][ro][d] << ";";
}
}
file<<std::endl;
}

for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

for(int d=0;d<GL_SZ_D;d++){
file << idx_ny[r][ro][d] << ";";
}
}
file<<std::endl;
}

file.close();


}

}


static void m_load(std::string path){



std::ifstream file;
file.open (path);

if(file.is_open()){

std::string line;

std::getline(file, line);
GL_SZ_R=atoi(line.c_str());

std::getline(file, line);
GL_SZ_O=atoi(line.c_str());

std::getline(file, line);
GL_SZ_D=atoi(line.c_str());

m_init();


for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){

std::string ak_val="";


while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_ppx[r][ro]=atof(ak_val.c_str());


}
}



for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){

std::string ak_val="";


while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_ppy[r][ro]=atof(ak_val.c_str());


}
}









for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){

std::string ak_val="";


while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_weight[r][ro]=atof(ak_val.c_str());


}
}


for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){

std::string ak_val="";


while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_used[r][ro]=atoi(ak_val.c_str());

}
}









for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){
for(int d=0;d<GL_SZ_D;d++){
std::string ak_val="";

while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_px[r][ro][d]=atof(ak_val.c_str());

}
}
}

for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){
for(int d=0;d<GL_SZ_D;d++){
std::string ak_val="";

while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_py[r][ro][d]=atof(ak_val.c_str());

}
}
}


for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){
for(int d=0;d<GL_SZ_D;d++){
std::string ak_val="";

while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_nx[r][ro][d]=atof(ak_val.c_str());

}
}
}


for(int r=0;r<GL_SZ_R;r++){
std::getline(file, line);
int line_idx=0;
for(int ro=0;ro<GL_SZ_O;ro++){
for(int d=0;d<GL_SZ_D;d++){
std::string ak_val="";

while(line[line_idx]!=';'){
ak_val+=line[line_idx];
line_idx++;
}
line_idx++;

idx_ny[r][ro][d]=atof(ak_val.c_str());

}
}
}


}


}


static void m_fast(cv::Mat &in){

cv::Scalar mea=cv::mean(in);

cv::Mat heat;
heat=cv::Mat::zeros(in.rows,in.cols,CV_32FC1);

for(int i=20;i<in.rows-20;i++){	//ROI Y
for(int j=5;j<in.cols-5;j++){	//ROI X

float val_r=0;
float val_rc=0;


for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

if(idx_used[r][ro]){

int ppx=idx_ppx[r][ro]+j;
int ppy=idx_ppy[r][ro]+i;


if(ppx>0 && ppy>0 && ppx<in.cols && ppy<in.rows){

if(mea[0]>in.at<float>(ppy,ppx)){
float pos=0;
float neg=0;
for(int k=0;k<GL_SZ_D;k++){
int px=idx_px[r][ro][k]+j;
int py=idx_py[r][ro][k]+i;

if(px>0 && py>0 && px<in.cols && py<in.rows){
pos+=in.at<float>(py,px);
}

px=idx_nx[r][ro][k]+j;
py=idx_ny[r][ro][k]+i;

if(px>0 && py>0 && px<in.cols && py<in.rows){
neg+=in.at<float>(py,px);
}
}




if(pos<neg*0.6){
//val_r+=(1.0f-in.at<float>(ppy,ppx));
val_r++;
}
}
val_rc++;


}

}
}





}




if(val_rc>0)
val_r/=val_rc;

// Threshold festlegen
//if (val_r > heat.at<float>(i, j) && val_r > 0.3)
if (val_r > heat.at<float>(i, j))
{
heat.at<float>(i,j)=val_r;
//qDebug() << val_r;
}





}
}



cv::normalize(heat, heat, 0, 1, cv::NORM_MINMAX, CV_32FC1);

//imshow("in",in);

//imshow("heat",heat);


heat.copyTo(in);


}


static void m_clear(){

if(GL_SZ_R>0 && GL_SZ_O>0 && GL_SZ_D>0){

for(int r=0;r<GL_SZ_R;r++){
for(int ro=0;ro<GL_SZ_O;ro++){

delete[] idx_px[r][ro];
delete[] idx_py[r][ro];

delete[] idx_nx[r][ro];
delete[] idx_ny[r][ro];

}
}


for(int r=0;r<GL_SZ_R;r++){
delete[] idx_ppx[r];
delete[] idx_ppy[r];

delete[] idx_px[r];
delete[] idx_py[r];

delete[] idx_nx[r];
delete[] idx_ny[r];

delete[] idx_weight[r];
delete[] idx_used[r];
}



delete[] idx_ppx;
delete[] idx_ppy;

delete[] idx_px;
delete[] idx_py;

delete[] idx_nx;
delete[] idx_ny;

delete[] idx_weight;
delete[] idx_used;
}

}


int64  ttt1,ttt2;

static cv::RotatedRect run_fast(cv::Mat input){

ttt1 = cv::getTickCount();

cv::Mat m;
cv::RotatedRect ellipse;
ellipse.size.height = 5;
ellipse.size.width = 5;

ellipse.center.x = -1;
ellipse.center.y = -1;


if(input.cols<100 || input.rows<100)
return ellipse;

if(input.channels()>1){
if(input.channels()==3)
cv::cvtColor(input, input, CV_BGR2GRAY);
else
return ellipse;
}



input.copyTo(m);
cv::equalizeHist(m,m);
m.convertTo(m,CV_32FC1);

cv::normalize(m, m, 0, 1, cv::NORM_MINMAX, CV_32FC1);
cv::GaussianBlur(m,m,cv::Size(3,3),1.5,1.5);



ttt2 = cv::getTickCount();

//qDebug()<< "Init:"<< double(ttt2-ttt1)/double(cv::getTickFrequency()) << endl;
//qDebug().flush();

// 1 Detection
ttt1 = cv::getTickCount();

m_fast(m);
//c_fast(m);
// imshow("m1",m);

for(int i=0;i<m.rows;i++){
for(int j=0;j<m.cols;j++){
m.at<float>(i,j)=1.0-m.at<float>(i,j);
}
}


ttt2 = cv::getTickCount();

//qDebug()<< "Detect1:" <<double(ttt2-ttt1)/double(cv::getTickFrequency()) << endl;
//qDebug().flush();



// 2 Detection
ttt1 = cv::getTickCount();

m_fast(m);
//c_fast(m);
// imshow("m2",m);


for(int i=0;i<m.rows;i++){
for(int j=0;j<m.cols;j++){
m.at<float>(i,j)=1.0-m.at<float>(i,j);
}
}

ttt2 = cv::getTickCount();

//qDebug()<<"Detect2:"<< (double(ttt2-ttt1)/double(cv::getTickFrequency())) << endl;




// 3 Detection
ttt1 = cv::getTickCount();

m_fast(m);
//c_fast(m);
// imshow("m3",m);

ttt2 = cv::getTickCount();
// qDebug()<<"Detect3:"<<double(ttt2-ttt1)/double(cv::getTickFrequency()) << endl;


// Selection
ttt1 = cv::getTickCount();


// Welcher Wert soll er mindestens haben
float max_val=0;
//float max_val = 90;
for(int i=8;i<m.rows-8;i++)
{
for(int j=8;j<m.cols-8;j++)
{

float ak_val=0;
for(int k1=-7;k1<=7;k1++)
{
for(int k2=-7;k2<7;k2++)
{
ak_val+=m.at<float>(i+k1,j+k2);
}
}


if(ak_val>max_val)
{
max_val=ak_val;
ellipse.center.x=j;
ellipse.center.y=i;
}

}
}



ttt2 = cv::getTickCount();

return ellipse;
}





}



// 1
/*

#include <iostream>
#include <fstream>
namespace ELSE{
int GL_SZ_R;
int GL_SZ_O;
int GL_SZ_D;

float **idx_ppx;
float **idx_ppy;
float ***idx_px;
float ***idx_py;
float ***idx_nx;
float ***idx_ny;


bool **idx_used;
float **idx_weight;


static void m_init() {

	int sz_r = GL_SZ_R;
	int sz_o = GL_SZ_O;
	int sz_d = GL_SZ_D;


	idx_ppx = new float*[sz_r];
	idx_ppy = new float*[sz_r];

	idx_px = new float**[sz_r];
	idx_py = new float**[sz_r];

	idx_nx = new float**[sz_r];
	idx_ny = new float**[sz_r];

	idx_weight = new float*[sz_r];
	idx_used = new bool*[sz_r];


	for (int r = 0; r<sz_r; r++) {
		idx_ppx[r] = new float[sz_o];
		idx_ppy[r] = new float[sz_o];

		idx_px[r] = new float*[sz_o];
		idx_py[r] = new float*[sz_o];

		idx_nx[r] = new float*[sz_o];
		idx_ny[r] = new float*[sz_o];


		idx_weight[r] = new float[sz_o];
		idx_used[r] = new bool[sz_o];
	}


	for (int r = 0; r<sz_r; r++) {
		for (int ro = 0; ro<sz_o; ro++) {

			idx_px[r][ro] = new float[sz_d];
			idx_py[r][ro] = new float[sz_d];

			idx_nx[r][ro] = new float[sz_d];
			idx_ny[r][ro] = new float[sz_d];

		}
	}

}


static void m_load(std::string path) {



	std::ifstream file;
	file.open(path);

	if (file.is_open()) {

		std::string line;

		std::getline(file, line);
		GL_SZ_R = atoi(line.c_str());

		std::getline(file, line);
		GL_SZ_O = atoi(line.c_str());

		std::getline(file, line);
		GL_SZ_D = atoi(line.c_str());

		m_init();


		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {

				std::string ak_val = "";


				while (line[line_idx] != ';') {
					ak_val += line[line_idx];
					line_idx++;
				}
				line_idx++;

				idx_ppx[r][ro] = atof(ak_val.c_str());


			}
		}



		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {

				std::string ak_val = "";


				while (line[line_idx] != ';') {
					ak_val += line[line_idx];
					line_idx++;
				}
				line_idx++;

				idx_ppy[r][ro] = atof(ak_val.c_str());


			}
		}









		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {

				std::string ak_val = "";


				while (line[line_idx] != ';') {
					ak_val += line[line_idx];
					line_idx++;
				}
				line_idx++;

				idx_weight[r][ro] = atof(ak_val.c_str());


			}
		}


		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {

				std::string ak_val = "";


				while (line[line_idx] != ';') {
					ak_val += line[line_idx];
					line_idx++;
				}
				line_idx++;

				idx_used[r][ro] = atoi(ak_val.c_str());

			}
		}









		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {
				for (int d = 0; d<GL_SZ_D; d++) {
					std::string ak_val = "";

					while (line[line_idx] != ';') {
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_px[r][ro][d] = atof(ak_val.c_str());

				}
			}
		}

		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {
				for (int d = 0; d<GL_SZ_D; d++) {
					std::string ak_val = "";

					while (line[line_idx] != ';') {
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_py[r][ro][d] = atof(ak_val.c_str());

				}
			}
		}


		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {
				for (int d = 0; d<GL_SZ_D; d++) {
					std::string ak_val = "";

					while (line[line_idx] != ';') {
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_nx[r][ro][d] = atof(ak_val.c_str());

				}
			}
		}


		for (int r = 0; r<GL_SZ_R; r++) {
			std::getline(file, line);
			int line_idx = 0;
			for (int ro = 0; ro<GL_SZ_O; ro++) {
				for (int d = 0; d<GL_SZ_D; d++) {
					std::string ak_val = "";

					while (line[line_idx] != ';') {
						ak_val += line[line_idx];
						line_idx++;
					}
					line_idx++;

					idx_ny[r][ro][d] = atof(ak_val.c_str());

				}
			}
		}


	}


}




cv::Mat CNM1 = cv::Mat::zeros(100, 100, CV_32FC1);
cv::Mat CNM2 = cv::Mat::zeros(100, 100, CV_32FC1);


static void m_fast(cv::Mat &in, int st_r, int en_r) {



	cv::Scalar mea = cv::mean(in);


	for (int i = 20; i<in.rows - 15; i++) {	//ROI Y //10 //10
		for (int j = 10; j<in.cols - 10; j++) {	//ROI X //5 //5

			CNM2.at<float>(i, j) = 0.0;


			float val_r = 0;
			float val_conn = 0;
			float val_rc = 0;


			//for(int r=0;r<GL_SZ_R;r++){
			for (int r = st_r; r<en_r; r++) {
				bool last = false;
				for (int ro = 0; ro<GL_SZ_O; ro++) {

					if (idx_used[r][ro]) {

						int ppx = idx_ppx[r][ro] + j;
						int ppy = idx_ppy[r][ro] + i;


						if (ppx>0 && ppy>0 && ppx<in.cols && ppy<in.rows) {


							//if(true || mea[0]>in.at<float>(ppy,ppx)){
							if (mea[0] * 0.5>in.at<float>(ppy, ppx)) {//works fine same results
								float pos = 0;
								float neg = 0;

								float mea_cnt = 0;

								float pos_o = 0;
								float neg_o = 0;

								for (int k = 0; k<GL_SZ_D; k++) {
									int px = idx_px[r][ro][k] + j;
									int py = idx_py[r][ro][k] + i;

									int npx = idx_nx[r][ro][k] + j;
									int npy = idx_ny[r][ro][k] + i;


									if (px>0 && py>0 && px<in.cols && py<in.rows &&
										npx>0 && npy>0 && npx<in.cols && npy<in.rows) {

										pos += in.at<float>(py, px);
										neg += in.at<float>(npy, npx);

										mea_cnt += mea[0];


									}




								}


								if (pos<neg*0.9 && pos<mea_cnt && neg<mea_cnt && last) {//0.9
									val_r += val_conn;
								}

								if (pos<neg*0.9 && pos<mea_cnt && neg<mea_cnt) {//0.9
									val_r++;
									val_conn++;
									last = true;
								}
								else {
									val_conn = 0;
									last = false;
								}





							}





							val_rc++;

						}

					}
				}







			}




			if (val_rc>0) {
				val_r /= val_rc;
				CNM2.at<float>(i, j) = val_r;
			}





		}
	}


	CNM2.copyTo(in);

}


static void m_clear() {

	if (GL_SZ_R>0 && GL_SZ_O>0 && GL_SZ_D>0) {

		for (int r = 0; r<GL_SZ_R; r++) {
			for (int ro = 0; ro<GL_SZ_O; ro++) {

				delete[] idx_px[r][ro];
				delete[] idx_py[r][ro];

				delete[] idx_nx[r][ro];
				delete[] idx_ny[r][ro];

			}
		}


		for (int r = 0; r<GL_SZ_R; r++) {
			delete[] idx_ppx[r];
			delete[] idx_ppy[r];

			delete[] idx_px[r];
			delete[] idx_py[r];

			delete[] idx_nx[r];
			delete[] idx_ny[r];

			delete[] idx_weight[r];
			delete[] idx_used[r];
		}



		delete[] idx_ppx;
		delete[] idx_ppy;

		delete[] idx_px;
		delete[] idx_py;

		delete[] idx_nx;
		delete[] idx_ny;

		delete[] idx_weight;
		delete[] idx_used;
	}

}



static cv::RotatedRect run_fast(cv::Mat input) {


	cv::RotatedRect ellipse;
	ellipse.size.height = 5;
	ellipse.size.width = 5;

	ellipse.center.x = -1;
	ellipse.center.y = -1;


	if (input.cols<1 || input.rows<1)
		return ellipse;

	if (input.channels()>1) {
		if (input.channels() == 3)
			cv::cvtColor(input, input, CV_BGR2GRAY);
		else
			return ellipse;
	}



	input.copyTo(CNM1);
	CNM1.convertTo(CNM1, CV_32FC1);

	cv::normalize(CNM1, CNM1, 0, 1, cv::NORM_MINMAX, CV_32FC1);

	m_fast(CNM1, 0, GL_SZ_R / 2);//8


	cv::boxFilter(CNM1, CNM1, -1, cv::Size(7, 7));

	float max_val = 0;
	for (int i = 20; i<CNM1.rows - 15; i++) {
		for (int j = 10; j<CNM1.cols - 10; j++) {


			if (CNM1.at<float>(i, j)>max_val) {
				max_val = CNM1.at<float>(i, j);
				ellipse.center.x = j;
				ellipse.center.y = i;
			}

		}
	}



	return ellipse;
}


}*/


#endif