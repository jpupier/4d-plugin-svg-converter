/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.c
 #	source generated by 4D Plugin Wizard
 #	Project : SVG Converter
 #	author : miyako
 #	2013/08/14
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(int32_t selector, PA_PluginParameters params)
{
	try
	{
		int32_t pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (int32_t pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
		case kDeinitPlugin :
		case kServerDeinitPlugin :			
			rsvg_cleanup();
			break;			
			
			// --- Convert Many
			
		case 1 :
			SVG_Convert_array(pResult, pParams);
			break;
			
			// --- Converter
			
		case 2 :
			SVG_Convert(pResult, pParams);
			break;
			
	}
}

static cairo_status_t rsvg_cairo_write_func (void *data, const unsigned char *bytes, unsigned int len)
{	
	if(len > 0)
	{
		C_BLOB *blob = (C_BLOB *)data;
		blob->addBytes(bytes, len);
	}
	
	return CAIRO_STATUS_SUCCESS;
}

static void rsvg_cairo_size_callback (int *width, int *height, gpointer data)
{
    RsvgDimensionData *dimensions = (RsvgDimensionData *)data;
    *width = dimensions->width;
    *height = dimensions->height;
}

void _getParams(ARRAY_LONGINT &keys, ARRAY_REAL &values,
				double *dpi_x,
				double *dpi_y,
				int *width,
				int *height,
				double *x_zoom,
				double *y_zoom,
				double *zoom, 
				int *keep_aspect_ratio
				){
	
	//default values
	
	*dpi_x = -1.0;
    *dpi_y = -1.0;
	*width = -1;
    *height = -1;
	*x_zoom = 1.0;
    *y_zoom = 1.0;		
	*zoom = 1.0;
	*keep_aspect_ratio = FALSE;	
	
	int option;
	double value;
	
	for(unsigned int i = 0; i < keys.getSize(); ++i) {
		
		option = keys.getIntValueAtIndex(i);
		value = values.getDoubleValueAtIndex(i);
		
		switch (option) {
				
			case SVG_DPI_X:
				*dpi_x = value;
				break;
				
			case SVG_DPI_Y:
				*dpi_y = value;
				break;
				
			case SVG_ZOOM_X:
				*x_zoom = value;
				break;	
				
			case SVG_ZOOM_Y:
				*y_zoom = value;
				break;	
				
			case SVG_ZOOM:
				*zoom = value;
				break;	
				
			case SVG_WIDTH:
				*width = value;
				break;
				
			case SVG_HEIGHT:
				*height = value;
				break;	
				
			case SVG_KEEP_ASPECT_RATIO:
				*keep_aspect_ratio = value;
				break;					
		}
		
	}
	
	if (*zoom != 1.0)
        *x_zoom = *y_zoom = *zoom;
	
}

// --------------------------------- Convert Many ---------------------------------


void SVG_Convert_array(sLONG_PTR *pResult, PackagePtr pParams)
{
	PA_Variable Param1;
	C_BLOB Param2_Out;
	C_LONGINT Param3_Format;
	ARRAY_LONGINT Param4_Keys;
	ARRAY_REAL Param5_Values;
	C_TEXT Param6_Color;
	C_TEXT Param7_Uri;
	C_LONGINT returnValue;
	
	Param3_Format.fromParamAtIndex(pParams, 3);
	Param4_Keys.fromParamAtIndex(pParams, 4);
	Param5_Values.fromParamAtIndex(pParams, 5);
	Param6_Color.fromParamAtIndex(pParams, 6);
	Param7_Uri.fromParamAtIndex(pParams, 7);
	
	// --- write the code of SVG_Convert_array here...
	
	int format = Param3_Format.getIntValue();
	
	CUTF8String color;
	Param6_Color.copyUTF8String(&color);
	
	CUTF8String uri;
	Param7_Uri.copyUTF8String(&uri);
	
	double dpi_x, dpi_y;
	int width, height;
	double x_zoom, y_zoom, zoom;
	int keep_aspect_ratio;
	
	_getParams(Param4_Keys, Param5_Values, &dpi_x, &dpi_y, &width, &height, &x_zoom, &y_zoom, &zoom, &keep_aspect_ratio);
	
	RsvgHandle *rsvg;
	cairo_surface_t *surface = NULL;
	cairo_t *cr = NULL;	
	
	RsvgDimensionData dimensions;
	GError *error = NULL;
	
	Param1 = *((PA_Variable*) pParams[0]);
	
	switch (Param1.fType) 
	{
		case eVK_ArrayPicture:
			break;
		case eVK_Undefined:
			PA_ClearVariable(&Param1);				
			Param1 = PA_CreateVariable(eVK_ArrayPicture);
			break;				
		default:
			break;
	}
	
	CUTF8String typeSvg = CUTF8String((const uint8_t *)".svg", 4);
	
	if(Param1.fType == eVK_ArrayPicture){
	
		if((format == SVG_OUTPUT_PDF) || (format == SVG_OUTPUT_PS)){
			
			rsvg_set_default_dpi_x_y (dpi_x, dpi_y);

			for(int i = 0; i <= Param1.uValue.fArray.fNbElements; ++i) {
				
				PA_Picture picture = PA_GetPictureInArray(Param1, i);
				
				unsigned int j;
				PA_ErrorCode err = eER_NoErr;
				
				j = 0;
				
				while (err == eER_NoErr){
					
					PA_Handle h = PA_NewHandle(0);	
					PA_Unistring u = PA_GetPictureData(picture, ++j, h);
					err = PA_GetLastError();
					
					if(err == eER_NoErr)
					{
						C_TEXT t;
						t.setUTF16String(&u);
						CUTF8String uti;
						t.copyUTF8String(&uti);
						CUTF8String typestring;
						
						size_t pos, found; 
						found = 0;
						
						for(pos = uti.find(';'); pos != CUTF8String::npos; pos = uti.find(';', found))
						{
							typestring = uti.substr(found, pos-found);
							found = pos + 1;
							
							if(!typestring.compare(typeSvg)){
								
								//found svg
								rsvg = rsvg_handle_new_from_data((const guint8 *)PA_LockHandle(h), PA_GetHandleSize(h), &error);
								
								if (rsvg) {
									
									if(uri.size())
										rsvg_handle_set_base_uri (rsvg, (const char *)uri.c_str());
									
									rsvg_handle_set_size_callback (rsvg, rsvg_cairo_size_callback, &dimensions, NULL);
									
									if(!surface) {
										
										struct RsvgSizeCallbackData size_data;
										rsvg_handle_get_dimensions (rsvg, &dimensions);
										
										if (width == -1 && height == -1) {
											size_data.type = RSVG_SIZE_ZOOM;
											size_data.x_zoom = x_zoom;
											size_data.y_zoom = y_zoom;
											size_data.keep_aspect_ratio = keep_aspect_ratio;
										}
										else if (x_zoom == 1.0 && y_zoom == 1.0) {
											if (width == -1 || height == -1) {
												size_data.type = RSVG_SIZE_WH_MAX;
												size_data.width = width;
												size_data.height = height;
												size_data.keep_aspect_ratio = keep_aspect_ratio;
											} else {
												size_data.type = RSVG_SIZE_WH;
												size_data.width = width;
												size_data.height = height;
												size_data.keep_aspect_ratio = keep_aspect_ratio;
											}
										} else {
											size_data.type = RSVG_SIZE_ZOOM_MAX;
											size_data.x_zoom = x_zoom;
											size_data.y_zoom = y_zoom;
											size_data.width = width;
											size_data.height = height;
											size_data.keep_aspect_ratio = keep_aspect_ratio;
										}
										
										_rsvg_size_callback (&dimensions.width, &dimensions.height, &size_data);
										
										switch (format) {
												
											case SVG_OUTPUT_PS:
												surface = cairo_ps_surface_create_for_stream (rsvg_cairo_write_func, 
																							  (void *)&Param2_Out,
																							  dimensions.width, dimensions.height);
												break;	
												
											case SVG_OUTPUT_PDF:
												surface = cairo_pdf_surface_create_for_stream (rsvg_cairo_write_func, 
																							   (void *)&Param2_Out,
																							   dimensions.width, 
																							   dimensions.height);
												break;		
												
										}
										
										if(surface) {
											
											cr = cairo_create(surface);
											
										}
																			
									}
									
									if(surface) {
									
										guint32 background_color = 0;
										
										if (color.size() && g_ascii_strcasecmp((const gchar *)color.c_str(), "none") != 0) {
											background_color = rsvg_css_parse_color((const char *)color.c_str(), FALSE);
											
											cairo_set_source_rgb (
																  cr, 
																  ((background_color >> 16) & 0xff) / 255.0, 
																  ((background_color >> 8) & 0xff) / 255.0, 
																  ((background_color >> 0) & 0xff) / 255.0);
											cairo_rectangle (cr, 0, 0, dimensions.width, dimensions.height);
											cairo_fill (cr);
										}
										
										rsvg_handle_render_cairo (rsvg, cr);
										
										switch (format) {
												
											case SVG_OUTPUT_PDF:
											case SVG_OUTPUT_PS:		
												cairo_show_page(cr);
												break;
										}

									}
									
									g_object_unref(rsvg);
									
								}else{
									returnValue.setIntValue(-1);//svg parse error
								}								
								break;
							}
							
						}
						
					}
					PA_DisposeHandle(h);
				}			
				
			}
			
			if(surface){
				cairo_destroy(cr);
				cairo_surface_destroy(surface);	
			}
			
		}else{
			returnValue.setIntValue(-3);//invalid export format	
		}
		
	}
	
	Param2_Out.toParamAtIndex(pParams, 2);	
	returnValue.setReturn(pResult);
}


// ----------------------------------- Converter ----------------------------------


void SVG_Convert(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_PICTURE Param1_In;
	C_BLOB Param2_Out;
	C_LONGINT Param3_Format;
	ARRAY_LONGINT Param4_Keys;
	ARRAY_REAL Param5_Values;
	C_TEXT Param6_Color;
	C_TEXT Param7_Uri;
	C_LONGINT returnValue;
	
	Param1_In.fromParamAtIndex(pParams, 1);
	Param3_Format.fromParamAtIndex(pParams, 3);
	Param4_Keys.fromParamAtIndex(pParams, 4);
	Param5_Values.fromParamAtIndex(pParams, 5);
	Param6_Color.fromParamAtIndex(pParams, 6);
	Param7_Uri.fromParamAtIndex(pParams, 7);

	// --- write the code of SVG_Convert here...
	
	int format = Param3_Format.getIntValue();
	
	CUTF8String color;
	Param6_Color.copyUTF8String(&color);
	
	CUTF8String uri;
	Param7_Uri.copyUTF8String(&uri);
		
	double dpi_x, dpi_y;
	int width, height;
	double x_zoom, y_zoom, zoom;
	int keep_aspect_ratio;
	
	_getParams(Param4_Keys, Param5_Values, &dpi_x, &dpi_y, &width, &height, &x_zoom, &y_zoom, &zoom, &keep_aspect_ratio);
	
	RsvgHandle *rsvg;
	cairo_surface_t *surface = NULL;
	
	RsvgDimensionData dimensions;
	GError *error = NULL;
	
	CUTF8String type = CUTF8String((const uint8_t *)".svg", 4);
	
	const guint8 *p = Param1_In.getBytesPtr(&type);
	
	if(p) {
		
		rsvg_set_default_dpi_x_y (dpi_x, dpi_y);
		
		rsvg = rsvg_handle_new_from_data(p, Param1_In.getBytesLength(&type), &error);

		if (rsvg) {
		
			if(uri.size())
				rsvg_handle_set_base_uri (rsvg, (const char *)uri.c_str());
			
			rsvg_handle_set_size_callback (rsvg, rsvg_cairo_size_callback, &dimensions, NULL);
			
			struct RsvgSizeCallbackData size_data;
			rsvg_handle_get_dimensions (rsvg, &dimensions);
			
			if (width == -1 && height == -1) {
                size_data.type = RSVG_SIZE_ZOOM;
                size_data.x_zoom = x_zoom;
                size_data.y_zoom = y_zoom;
                size_data.keep_aspect_ratio = keep_aspect_ratio;
            }
			else if (x_zoom == 1.0 && y_zoom == 1.0) {
                if (width == -1 || height == -1) {
                    size_data.type = RSVG_SIZE_WH_MAX;
                    size_data.width = width;
                    size_data.height = height;
                    size_data.keep_aspect_ratio = keep_aspect_ratio;
                } else {
                    size_data.type = RSVG_SIZE_WH;
                    size_data.width = width;
                    size_data.height = height;
                    size_data.keep_aspect_ratio = keep_aspect_ratio;
                }
            } else {
                size_data.type = RSVG_SIZE_ZOOM_MAX;
                size_data.x_zoom = x_zoom;
                size_data.y_zoom = y_zoom;
                size_data.width = width;
                size_data.height = height;
                size_data.keep_aspect_ratio = keep_aspect_ratio;
            }
			
			_rsvg_size_callback (&dimensions.width, &dimensions.height, &size_data);
			
			switch (format) {
					
				case SVG_OUTPUT_PNG:
					surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, dimensions.width, dimensions.height);
					break;
			
				case SVG_OUTPUT_PS:
					surface = cairo_ps_surface_create_for_stream (rsvg_cairo_write_func, 
																  (void *)&Param2_Out,
																  dimensions.width, dimensions.height);
					break;	
					
				case SVG_OUTPUT_SVG:
					surface = cairo_svg_surface_create_for_stream (rsvg_cairo_write_func, 
																   (void *)&Param2_Out,
																   dimensions.width, dimensions.height);
					break;	
					
				case SVG_OUTPUT_PDF:
					surface = cairo_pdf_surface_create_for_stream (rsvg_cairo_write_func, 
																   (void *)&Param2_Out,
																   dimensions.width, 
																   dimensions.height);
					break;		

			}
			
			if(surface) {
				
				cairo_t *cr = NULL;	
				
				cr = cairo_create(surface);
				
				guint32 background_color = 0;
				if (color.size() && g_ascii_strcasecmp((const gchar *)color.c_str(), "none") != 0) {
					background_color = rsvg_css_parse_color((const char *)color.c_str(), FALSE);
					
					cairo_set_source_rgb (
										  cr, 
										  ((background_color >> 16) & 0xff) / 255.0, 
										  ((background_color >> 8) & 0xff) / 255.0, 
										  ((background_color >> 0) & 0xff) / 255.0);
					cairo_rectangle (cr, 0, 0, dimensions.width, dimensions.height);
					cairo_fill (cr);
				}
				
				rsvg_handle_render_cairo (rsvg, cr);
				
				switch (format) {
					case SVG_OUTPUT_PNG:
						cairo_surface_write_to_png_stream (surface, rsvg_cairo_write_func, (void *)&Param2_Out);
						break;	
						
					case SVG_OUTPUT_SVG:
					case SVG_OUTPUT_PDF:
					case SVG_OUTPUT_PS:		
						cairo_show_page(cr);
						break;
				}
				
				cairo_destroy(cr);	
				cairo_surface_destroy(surface);	
				
			}else{
				returnValue.setIntValue(-3);//invalid export format	
			}

			g_object_unref(rsvg);
		
		}else{
			returnValue.setIntValue(-1);//svg parse error
		}
	}else{
		returnValue.setIntValue(-2);//image is not svg
	}
	
	Param2_Out.toParamAtIndex(pParams, 2);
	returnValue.setReturn(pResult);
}
