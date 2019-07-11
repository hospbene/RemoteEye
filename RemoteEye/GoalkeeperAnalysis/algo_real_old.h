
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>


namespace ELSE{


	
static void gen_blob_neu2(int rad, cv::Mat *all_mat, cv::Mat *all_mat_neg){


		int len=1+(4*rad);
		int c0=rad*2;
		float negis=0;
		float posis=0;

		*all_mat = cv::Mat::zeros(len, len, CV_32FC1);
		*all_mat_neg = cv::Mat::zeros(len, len, CV_32FC1);
		


		float *p, *p_neg;
		for(int i=-rad*2;i<=rad*2;i++){ //height
			p=all_mat->ptr<float>(c0+i);

			for(int j=-rad*2;j<=rad*2;j++){

				if(i<-rad || i>rad){ //pos
					p[c0+j]=1;
					posis++;

				}else{ //neg

					int sz_w=sqrt( float(rad*rad) - float(i*i) );

					if(abs(j)<=sz_w){
						p[c0+j]=-1;
						negis++;
					}else{
						p[c0+j]=1;
						posis++;
					}

				}

			}
		}



	
	for(int i=0;i<len;i++){
		p=all_mat->ptr<float>(i);
		p_neg=all_mat_neg->ptr<float>(i);

		for(int j=0;j<len;j++){

			if(p[j]>0){
				p[j]=1.0/posis;
				p_neg[j]=0.0;
			}else{
				p[j]=-1.0/negis;
				p_neg[j]=1.0/negis;
			}

		}
	}


	for(int i=0;i<len/2;i++){
		p=all_mat->ptr<float>(i);
		p_neg=all_mat_neg->ptr<float>(i);

		for(int j=0;j<len;j++){

			p[j]=0.0;
			p_neg[j]=0.0;

		}
	}




}

static void rays(cv::Mat in,cv::RotatedRect &el, int sz){

	
	cv::Mat x = cv::Mat::zeros(in.rows, in.cols, CV_32FC1);
	cv::Mat y = cv::Mat::zeros(in.rows, in.cols, CV_32FC1);
	cv::Mat m = cv::Mat::zeros(in.rows, in.cols, CV_32FC1);

	int stx=el.center.x-(sz*1.5);
	int sty=el.center.y-(sz*1.5);
	int etx=el.center.x+(sz*1.5);
	int ety=el.center.y+(sz*1.5);

	if(stx<=0)
		stx=1;
	
	if(sty<=0)
		sty=1;
	
	if(etx>=in.cols)
		etx=in.cols-1;
	
	if(ety>=in.rows)
		ety=in.rows-1;


	for(int i=sty; i<ety;i++){
		for(int j=stx; j<etx;j++){

			x.at<float>(i,j)+= in.at<float>(i-1,j-1)-in.at<float>(i-1,j+1);
			x.at<float>(i,j)+= 2*(in.at<float>(i,j-1)-in.at<float>(i,j+1));
			x.at<float>(i,j)+= in.at<float>(i+1,j-1)-in.at<float>(i+1,j+1);


			y.at<float>(i,j)+= in.at<float>(i-1,j-1)-in.at<float>(i+1,j-1);
			y.at<float>(i,j)+= 2*(in.at<float>(i-1,j)-in.at<float>(i+1,j));
			y.at<float>(i,j)+= in.at<float>(i-1,j+1)-in.at<float>(i+1,j+1);



			m.at<float>(i,j)=sqrt(pow(x.at<float>(i,j),2)+pow(y.at<float>(i,j),2));
			if(m.at<float>(i,j)>0){
				x.at<float>(i,j)=x.at<float>(i,j)/m.at<float>(i,j);
				y.at<float>(i,j)=y.at<float>(i,j)/m.at<float>(i,j);
			}else{
				x.at<float>(i,j)=0;
				y.at<float>(i,j)=0;
			}

		}
	}

	
	cv::Mat erg = cv::Mat::zeros(in.rows, in.cols, CV_32FC1);

	
	for(int i=sty; i<ety;i++){
		for(int j=stx; j<etx;j++){


			
			for(int ki=sty; ki<ety;ki++){
				for(int kj=stx; kj<etx;kj++){


					float dist=sqrt(pow(float(ki-i),2) + pow(float(kj-j),2));
                    if(dist>0){
                        float dy=(ki-i)/dist;
                        float dx=(kj-j)/dist;
                    
                        erg.at<float>(i,j)+= pow((1.0-in.at<float>(ki,kj)),2) * pow(dx*x.at<float>(ki,kj) + dy*y.at<float>(ki,kj),2);
					}


				}
			}





		}
	}


	cv::blur(erg,erg,cv::Size(5,5));



	float val_max=0;
	float val_max_x=0;
	float val_max_y=0;


	for(int i=1; i<erg.rows-1;i++){
		for(int j=1; j<erg.cols-1;j++){

			if(erg.at<float>(i,j)>val_max){
				val_max=erg.at<float>(i,j);
				val_max_x=j;
				val_max_y=i;

			}
		}
	}

	el.center.x=val_max_x;
	el.center.y=val_max_y;

	
	/*
	cv::normalize(x, x, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	cv::normalize(y, y, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	cv::normalize(m, m, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	*/
	//cv::normalize(erg, erg, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	//imshow("mappvote",erg);
	

}

static cv::RotatedRect run(cv::Mat input_img1, cv::Mat input_img2, int size_el){

	cv::RotatedRect ellipse;
	ellipse.size.height = size_el;
	ellipse.size.width = size_el;

	ellipse.center.x = -1;
	ellipse.center.y = -1;

	if(input_img1.channels()==3)
		cvtColor(input_img1, input_img1, CV_BGR2GRAY);

	if (input_img2.channels() == 3)
		cvtColor(input_img2, input_img2, CV_BGR2GRAY);

	cv::Mat cp_i, cp_d;

	cv::Scalar m1=cv::mean(input_img1);
	cv::Scalar m2=cv::mean(input_img2);



	if(m1[0]<m2[0]){
		input_img1.copyTo(cp_i);
	}else{
		input_img2.copyTo(cp_i);
	}

	if (cp_i.cols < 50 || cp_i.rows < 50)
		return ellipse;

	//cv::imshow("input", cp_i);




	cv::medianBlur(cp_i,cp_i,3);

	//cv::imshow("input2", cp_i);
	cp_i.convertTo(cp_i, CV_32FC1);

	cv::normalize(cp_i, cp_i, 0, 1, cv::NORM_MINMAX, CV_32FC1);



	//cv::imshow("input3", cp_i);
	//cv::waitKey(1);

	cp_d=cv::Mat::zeros(cp_i.size(),CV_32FC1);

	

	for(int i=1; i<cp_i.rows-1;i++){
		for(int j=1; j<cp_i.cols-1;j++){

			float v1=float(cp_i.at<float>(i,j));
			float cnt=1;

			for(int k1=-1; k1<=1;k1++){
				for(int k2=-1; k2<=1;k2++){

					if(cp_i.at<float>(i,j)>float(cp_i.at<float>(i+k1,j+k2))){
						v1+=float(cp_i.at<float>(i+k1,j+k2));
						cnt++;
					}
				}
			}
			
			v1/=cnt;
			cp_d.at<float>(i,j)= log(1.0/pow(v1+0.001,4));


			//cp_i1.at<float>(i,j)=1.0/pow(v1+0.001,1);
			//cp_i2.at<float>(i,j)=1.0/pow(v2+0.001,1);

		}
	}

	
	cv::normalize(cp_i, cp_i, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	cv::normalize(cp_d, cp_d, 0, 1, cv::NORM_MINMAX, CV_32FC1);



	
	cv::Mat blob_mat, blob_mat_neg;
	gen_blob_neu2(size_el, &blob_mat, &blob_mat_neg);

	cv::Mat result, result_neg;
	
	cv::Mat erg = cv::Mat::zeros(cp_d.rows, cp_d.cols, CV_32FC1);

	filter2D(cp_d, result, -1 , blob_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );

	
	float * p_res, *p_neg_res, *p_erg, *p_in, *p_out, *p_inout;
	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
			if(p_res[j]>0)
				p_res[j]=0;
			else
				p_res[j]=-p_res[j];

		}
	}

	filter2D(cp_d, result_neg, -1 , blob_mat_neg, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );

	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);
		p_neg_res=result_neg.ptr<float>(i);
		p_erg=erg.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
				p_erg[j]=(p_neg_res[j])*(p_res[j]);
		}
	}


	//qDebug() << erg;
	
	cv::normalize(erg, erg, 0, 1, cv::NORM_MINMAX, CV_32FC1);

	/*
	cv::imshow("img1",cp_i);
	cv::imshow("cp_d",cp_d);
	cv::imshow("erg",erg);
	*/

	/*
	cv::normalize(blob_mat, blob_mat, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	cv::normalize(blob_mat_neg, blob_mat_neg, 0, 1, cv::NORM_MINMAX, CV_32FC1);
	cv::imshow("blob_mat",blob_mat);
	cv::imshow("blob_mat_neg",blob_mat_neg);
	*/

	
	float val_max=0;
	float val_max_x=0;
	float val_max_y=0;
	float max_cnt=0;


	for(int i=1; i<erg.rows-1;i++)
	{
		for(int j=1; j<erg.cols-1;j++)
		{

			if(erg.at<float>(i,j)>val_max*0.9)
			{
				
				if(erg.at<float>(i,j)>val_max*1.1)
				{
					val_max=erg.at<float>(i,j);
					val_max_x=j;
					val_max_y=i;
					max_cnt=1;
				}
				else
				{
					val_max_x+=j;
					val_max_y+=i;
					max_cnt++;
				}

			}
		}
	}


	val_max_x/=max_cnt;
	val_max_y/=max_cnt;


	
	ellipse.center.x = val_max_x;
	ellipse.center.y = val_max_y;
	
	
	rays(cp_i,ellipse,size_el);

	/*
	cv::imshow("img1",cp_i);
	cv::imshow("cp_d",cp_d);
	cv::imshow("erg",erg);
	
	cv::normalize(input_img2, input_img2, 0, 255, cv::NORM_MINMAX, CV_8UC1);
	cv::ellipse(input_img2,ellipse,cv::Scalar(255,255,255,255),2);
	cv::imshow("input_img2",input_img2);

	cv::waitKey(1000);
	*/

	return ellipse;
	


	
}




}

