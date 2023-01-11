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

   	auto getClassName() const -> FString  override  { return "RLine";} 
	FLineEdit Product{this};
	FLineEdit Quantity{this};
	FLineEdit UnitPrice{this};
	FLineEdit TotalPrice{this};
	int RowId;

	void initLayout() override 
	{
		Product.unsetShadow();
		Product.setAlignment(Align::Left);
		Product.addCallback("focus-out",this,&RLine::cb_uppercase,&Product);
		Quantity.unsetShadow();
		Quantity.setInputFilter("[0-9]");
		Quantity.setAlignment(Align::Right);
		UnitPrice.unsetShadow();
		UnitPrice.setInputFilter("[.0-9]");
		UnitPrice.setAlignment(Align::Right);
		UnitPrice.addCallback("focus-out",this,&RLine::cb_setprecision,&UnitPrice);
		TotalPrice.unsetShadow();
		TotalPrice.setInputFilter("[.0-9]");
		TotalPrice.setAlignment(Align::Right);
		TotalPrice.addCallback("focus-out",this,&RLine::cb_setprecision,&TotalPrice);
		TotalPrice.addCallback("focus-in",this,&RLine::cb_calculate,&TotalPrice);
		this->redraw();
	}
	void draw() override
	{
		int posx{2};
		int posy{1};
		int gap=2;
		size_t sizex = 1;
		size_t sizey = 1;
		const auto& parent = getParentWidget();
		auto totalwidth = parent->getWidth()-gap ;
		sizex = int((totalwidth/5)-gap);
		Product.setGeometry(FPoint{posx,posy},FSize{sizex*2,sizey});
		posx += sizex * 2 + gap; 
		Quantity.setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		posx += sizex + gap;
		UnitPrice.setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		posx += sizex + gap; 
		TotalPrice.setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
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

	void onFocusIn (FFocusEvent* in_ev) override
	{
		// Sets the focus to a child widget if it exists
		if ( ! hasChildren() ) FWidget::onFocusIn(in_ev);
		if ( in_ev->getFocusType() == FocusTypes::NextWidget ) { focusFirstChild() ? in_ev->accept() : in_ev->ignore(); }
		else if ( in_ev->getFocusType() == FocusTypes::PreviousWidget ) { focusLastChild() ? in_ev->accept() : in_ev->ignore(); }
	}

	//Change the behaviour when a child loses its focus
	//Will be used to calculate to totalprice of the receipt
	void onChildFocusOut (FFocusEvent * out_ev) override
	{
		const auto& focus = FWidget::getFocusWidget();
		if ( out_ev->getFocusType() == FocusTypes::NextWidget )
			{
			const auto& last_widget = getLastFocusableWidget(getChildren());
			if ( focus != last_widget ) return;
			out_ev->accept();
			focusNextChild();
			emitCallback("end-of-row");
			}
		else if ( out_ev->getFocusType() == FocusTypes::PreviousWidget )
			{
			const auto& first_widget = getFirstFocusableWidget(getChildren());
			if ( focus != first_widget ) return;
			out_ev->accept();
			focusPrevChild();
			}
	}

	//copy past from final::fwidget.cpp and added some debugging
	auto searchForwardForWidget ( const FWidget* parent , const FWidget* widget ) const -> FObjectList::const_iterator
	{
		auto iter = parent->cbegin();
		const auto last = parent->cend();
		while ( iter != last )  // Search forward for this widget
			{
			if ( ! (*iter)->isWidget() )  // Skip non-widget elements
				{
				++iter;
				continue;
				}
			if ( static_cast<FWidget*>(*iter) == widget )
				{
				break;  // Stop search when we reach this widget
				}
			++iter;
			}
		return iter;
	}

	//Overriding  this function to get out of the ScrollView by calling the grandparent when reaching last RLine
	//Probably there is an more elegant way to do this
	auto focusNextChild() -> bool
	{
		if ( isDialogWidget() || ! hasParent() ) return false;
		const auto& parent = getParentWidget();
		if ( ! parent || ! parent->hasChildren() || parent->numOfFocusableChildren() < 1 ) return false;
		FWidget* next = nullptr;
		constexpr auto ft = FocusTypes::NextWidget;
		auto iter = searchForwardForWidget(parent, this);
		auto iter_of_this_widget = iter;
		do  // Search the next focusable widget
			{
			++iter;
			if ( iter == parent->cend() )
				{
				iter = parent->cbegin();
				const auto& grandparent = parent->getParentWidget();
				parent->setFocus();
				grandparent->focusNextChild();
				return parent ? parent->setFocus(true,ft) : false;
				}
			if ( (*iter)->isWidget() ) next = static_cast<FWidget*>(*iter);
		} while ( iter != iter_of_this_widget && canReceiveFocus(next) );
		// Change focus to the next widget and return true if successful
		return next ? next->setFocus (true, ft) : false;
		}

		auto canReceiveFocus (const FWidget* widget) const -> bool
		{
		return ! widget || ! widget->isEnabled() || ! widget->acceptFocus() || ! widget->isShown() || widget->isWindowWidget();
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
			this->setResizeable();
			}
		int maxlines = 25;

		FScrollView frame_ReceiptDetail{this};
		std::vector<RLine*> Lines;

		FLineEdit ReceiptTotal{this};
		
		FButton BtnTotal{this};
		FButton BtnPrint{this};
		FButton BtnQuit{this};
		
		//Set Headers for the columns;
		FLabel* ColumnName1 = new FLabel{&frame_ReceiptDetail}; 
		FLabel* ColumnName2 = new FLabel{&frame_ReceiptDetail}; 
		FLabel* ColumnName3 = new FLabel{&frame_ReceiptDetail}; 
		FLabel* ColumnName4 = new FLabel{&frame_ReceiptDetail}; 

		void initLayout() override
		{
		frame_ReceiptDetail.setText("Register");
		frame_ReceiptDetail.setPos(FPoint{1,2});
		frame_ReceiptDetail.setSize(FSize{80,30});
		frame_ReceiptDetail.setScrollSize(FSize{200,200});
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

		this->redraw();
		}
	void insertLines()
	{
		for(int i=1;i<=maxlines;i++)
		{
		 auto l = new RLine(&frame_ReceiptDetail);
		l->RowId = i;
		l->addCallback("end-of-row",this,&ReceiptForm::cb_rowchange);
		l->show();
		Lines.push_back(l);
		}
	}

	void addHeaderLabels()
	{
		ColumnName1->setText("Product");
		ColumnName2->setText("Quantity");
		ColumnName3->setText("Unit Price");
		ColumnName4->setText("Total Price");
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

//This function uses iterators to loop over the vector
	void cb_rowchange()
	{
		cb_sum(); //callback to cb_sum to update the total of the receipt , this makes the Total button unnecessary
	}

	void centerDialog()
	{
		auto x = int((getDesktopWidth() - getWidth()) / 2);
		auto y = int((getDesktopHeight() - getHeight()) / 2);
		checkMinValue(x);
		checkMinValue(y);
		setPos (FPoint{x, y}, false);
	}

	void checkMinValue (int& n)
	{
		if ( n < 1 ) n = 1 ;
	}
	
	void adjustSize() override
	{
	FDialog::adjustSize();
	this->centerDialog();
	this->adjustWidgets();	
	}

	void adjustWidgets()
	{
		auto bx = int(getWidth() - 5);
		auto by = int(getHeight() - 6);
		frame_ReceiptDetail.setWidth(bx);
		frame_ReceiptDetail.setHeight(by);
		frame_ReceiptDetail.setScrollWidth(bx - 3);
		frame_ReceiptDetail.setScrollHeight(100);
		frame_ReceiptDetail.setColor(FColor::Blue,FColor::White);
		frame_ReceiptDetail.clearArea();
		

		int posx=3;	
		int posy=1;
		int gap=2;
		unsigned long int sizex = 1;
		unsigned long int sizey = 1;
		auto totalwidth = frame_ReceiptDetail.getWidth()-gap ;
		sizex = int((totalwidth/5-gap));
		
		ColumnName1->setGeometry(FPoint{posx,posy},FSize{sizex*2,sizey});
		posx+=sizex*2+gap;
		ColumnName2->setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		posx+=sizex+gap;
		ColumnName3->setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		posx+=sizex+gap;
		ColumnName4->setGeometry(FPoint{posx,posy},FSize{sizex,sizey});
		posy = 2;
		for ( auto l : Lines)
		{
			l->setGeometry(FPoint{1,posy},FSize{totalwidth-5,1});
			posy+=2;
		}
	//Align at the bottom the following widgets
		by = int(getHeight()-3);
		ReceiptTotal.setY(by);
		BtnTotal.setY(by);
		BtnPrint.setY(by);
		BtnQuit.setY(by);

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
