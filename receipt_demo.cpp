// Little Demo Applicition created to test the finalcut framework
// Created by Wim Stockman on 2022-12-14 wim@thinkerwim.org


#include <final/final.h>
#include <fstream>

using namespace finalcut;
//--------------------------------------------------------------------------------------------------
//          Receipt Line Class
//--------------------------------------------------------------------------------------------------
class RLine : public FWidget
{
	public: explicit RLine (FWidget* parent):FWidget{parent}{}

	FLineEdit Product{this};
	FLineEdit Quantity{this};
	FLineEdit UnitPrice{this};
	FLineEdit TotalPrice{this};

	void initLayout() override 
	{
		int posx = 1;
		int posy = 1;
		unsigned long int sizex = 10;
		unsigned long int sizey = 1;
		Product.setGeometry(FPoint{posx,posy},FSize{sizex*2,sizey});
		Product.unsetShadow();
		Product.setAlignment(Align::Left);
		Product.addCallback("focus-out",this,&RLine::cb_uppercase,&Product);
		posx+=22;
		Quantity.setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		Quantity.unsetShadow();
		Quantity.setInputFilter("[0-9]");
		Quantity.setAlignment(Align::Right);
		posx+=12;
		UnitPrice.setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		UnitPrice.unsetShadow();
		UnitPrice.setInputFilter("[.0-9]");
		UnitPrice.setAlignment(Align::Right);
		UnitPrice.addCallback("focus-out",this,&RLine::cb_setprecision,&UnitPrice);
		posx+=12;
		TotalPrice.setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		TotalPrice.unsetShadow();
		TotalPrice.setInputFilter("[.0-9]");
		TotalPrice.setAlignment(Align::Right);
		TotalPrice.addCallback("focus-out",this,&RLine::cb_setprecision,&TotalPrice);
		TotalPrice.addCallback("focus-in",this,&RLine::cb_calculate,&TotalPrice);
		posx+=12;
	}

	void cb_calculate(FLineEdit* e)
	{
		if (Quantity.getText() != "" && UnitPrice.getText() != "")
		{
			FString fmt = "";
		 	TotalPrice = fmt.sprintf("%.2f",Quantity.getText().toFloat() * UnitPrice.getText().toFloat());
		}

	}

	//Callback function to set the precision of the LineEdit to 2 decimals
	void cb_setprecision(FLineEdit* e)
	{
		FString fmt = "";
		FString text = e->getText();
		if (text != "") e->setText(fmt.sprintf("%.2f",text.toFloat()));
	}

	//Callback function to set the LineEdit Text to UpperCase
	void cb_uppercase(FLineEdit* e)
	{
		e->setText(e->getText().toUpper());
	}
	//Change the behaviour when a child loses its focus
	//Will be used to calculate to totalprice of the receipt
	void onChildFocusOut (FFocusEvent * out_ev) override
	{
		const auto focus_Widget = FWidget::getFocusWidget();
  		if ( out_ev->getFocusType() == FocusTypes::NextWidget )
		{	
		    const auto& last_widget = getLastFocusableWidget(getChildren());
    			if ( focus_Widget == last_widget )
			{
				out_ev->accept();
				emitCallback("end-of-row");
			}
		}
		else if ( out_ev->getFocusType() == FocusTypes::PreviousWidget )
		{
			const auto& first_widget = getFirstFocusableWidget(getChildren());
			if ( focus_Widget == first_widget )
			{
				out_ev->accept();
				focusPrevChild();
			}
		}
	}
};
//--------------------------------------------------------------------------------------------------
//End of Class RLine
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//          Receipt Form Class
//--------------------------------------------------------------------------------------------------
class ReceiptForm : public FDialog
{
	public:
		explicit ReceiptForm (FWidget *parent):FDialog{parent} 
			{
			posx=8; posy=1;
			sizex=30; sizey=1;
			}
		int maxlines = 25;

		int posx,posy;
		unsigned long int sizex,sizey;
		FScrollView frame_ReceiptDetail{this};
		std::vector<RLine*> Lines;

		FLineEdit ReceiptTotal{this};
		
		FButton BtnTotal{this};
		FButton BtnPrint{this};
		FButton BtnQuit{this};
		
		//Set Headers for the columns;
		FLabel ColumnName1{&frame_ReceiptDetail}; 
		FLabel ColumnName2{&frame_ReceiptDetail}; 
		FLabel ColumnName3{&frame_ReceiptDetail}; 
		FLabel ColumnName4{&frame_ReceiptDetail}; 

		void initLayout() override
		{
		frame_ReceiptDetail.setText("Register");
		frame_ReceiptDetail.setPos(FPoint{1,2});
		frame_ReceiptDetail.setSize(FSize{80,30});
		frame_ReceiptDetail.setScrollSize(FSize{70,100});
		frame_ReceiptDetail.setColor(FColor::Blue,FColor::White);
		frame_ReceiptDetail.clearArea();

		addHeaderLabels();
		insertLines();

		ReceiptTotal.setGeometry(FPoint{8,32},FSize{20,1});
		ReceiptTotal.setLabelText("Total");
		ReceiptTotal.setAlignment(Align::Right);

		BtnTotal = "&Total";
		BtnTotal.setGeometry(FPoint{38,32},FSize{14,1});
		BtnTotal.addCallback("clicked",this,&ReceiptForm::cb_sum);
		

		BtnPrint = "&Print";
		BtnPrint.setGeometry(FPoint{54,32},FSize{14,1});
		BtnPrint.addCallback("clicked",this,&ReceiptForm::cb_print);

		BtnQuit = "&Quit";
		BtnQuit.setGeometry(FPoint{70,32},FSize{14,1});
		BtnQuit.addCallback("clicked",this,&ReceiptForm::cb_quit);

		Lines[0]->Product.setFocus();
		this->redraw();
		}
	void insertLines()
	{
		for(int i=1;i<=maxlines;i++)
		{
		 auto l = new RLine(&frame_ReceiptDetail);
		l->addCallback("end-of-row",this,&ReceiptForm::cb_rowchange);
		l->setGeometry(FPoint{2,2*i+1},FSize{135,1});
		l->show();
		Lines.push_back(l);
		}
	}

	void addHeaderLabels()
	{
		int posx=1;	
		int posy=1;
		unsigned long int sizex = 10;
		unsigned long int sizey = 1;
		ColumnName1.setText("Product");
		ColumnName2.setText("Quantity");
		ColumnName3.setText("Unit Price");
		ColumnName4.setText("Total Price");

		ColumnName1.setGeometry(FPoint{posx,posy},FSize{sizex*2,sizey});
		posx+=sizex*2+2;
		ColumnName2.setGeometry(FPoint{posx,posy},FSize{sizex*2,sizey});
		posx+=sizex+2;
		ColumnName3.setGeometry(FPoint{posx,posy},FSize{sizex*2,sizey});
		posx+=sizex+2;
		ColumnName4.setGeometry(FPoint{posx,posy},FSize{sizex*2,sizey});
	}
	
	void cb_quit()
	{
		this->quit();
	}      
	void cb_sum()
	{
		float sum = 0;
		for (RLine* l: Lines)
		{
			FString fieldvalue = l->TotalPrice.getText();
			if (fieldvalue != "") { sum += fieldvalue.toFloat(); }
		}
		FString fmt = "";
		ReceiptTotal = fmt.sprintf("%.2f",sum);
		ReceiptTotal.redraw();
	}
	
	void cb_print()
	{
		std::string filename("receipt.groff");
		std::ofstream file_out;
		file_out.open(filename,std::ios_base::out);
		for (RLine* l : Lines) 
		{
			FString fieldvalue = l->Product.getText();
			if (fieldvalue != "") 
			{
				file_out << ".RECEIPTLINE "
					<< " \"" << l->Product.getText() << "\""
					<< " \"" << l->Quantity.getText() << "\""
					<< " \"" << l->UnitPrice.getText() << "\""
					<< " \"" << l->TotalPrice.getText() << "\""
					<< std::endl ;
			}
		}
		file_out << ".RECEIPTTOTAL "
			<< "\"" << this->ReceiptTotal.getText() << "\""
			<< std::endl ;
		file_out.close();
		std::system("./printreceipt.sh");
	}	
// Same cb_rowchange function but this one uses index to loop over the vector
// Just to show the possibility
//	void cb_rowchange()
//	{
//		long unsigned int j = Lines.size();
//		for (long unsigned int i =0 ; i < j ; i++)
//		{
//			if(Lines[i]->TotalPrice.hasFocus())
//			{ 	
//				if ( i == j-1)
//				{
//				Lines[0]->Product.setFocus();
//				Lines[0]->redraw();
//				i=j;
//				}
//				else
//				{
//				Lines[(i+1)]->Product.setFocus();
//				Lines[(i+1)]->redraw();
//				i=j;
//				}
//			}
//		}
//		cb_sum();
//			
//	}
//

//This function uses iterators to loop over the vector
	void cb_rowchange()
	{
	auto j = Lines.begin();
		for (auto i = Lines.begin() ; i != Lines.end() ; ++i)
		{
			if ((*i)->TotalPrice.hasFocus())
			{ 	
				if ( i == (Lines.end())-1)
				{
				(*Lines.begin())->Product.setFocus();
				(*j)->redraw();
				break;
				}
				else
				{
				i++;
				(*i)->Product.setFocus();
				(*i)->redraw();
				break;
				}
			}
		}
		
		cb_sum(); //callback to cb_sum to update the total of the receipt , this makes the Total button unnecessary
			
	}
	
};


int main (int argc, char* argv[])
{
	//create the Appliction object
	finalcut::FApplication app{argc,argv};
	app.initTerminal();

	//Create dialog box
	ReceiptForm dgl{&app};
	dgl.setText("Receipt Register");
	dgl.setGeometry(FPoint{1,1},FSize{100,40});
	dgl.redraw();
	FWidget::setMainWidget(&dgl);
	dgl.show();
	return app.exec();

}
