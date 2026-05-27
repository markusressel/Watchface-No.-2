#include "layer_factory.h"

LayerBuilder layer_builder(Layer *parent, LayerLayout layout) {
	GRect parent_bounds = layer_get_bounds(parent);
	return (LayerBuilder){
		.parent = parent,
		.bounds = GRect(
			layout.x,
			layout.y,
			parent_bounds.size.w - layout.width_margin,
			layout.height
		),
	};
}

TextLayer *layer_factory_create_text_layer(
	LayerBuilder builder,
	TextLayerStyle style
) {
	TextLayer *layer = text_layer_create(builder.bounds);

	text_layer_set_background_color(layer, style.background_color);
	text_layer_set_text_color(layer, style.text_color);
	text_layer_set_font(layer, style.font);
	text_layer_set_text_alignment(layer, style.alignment);

	layer_add_child(builder.parent, text_layer_get_layer(layer));

	return layer;
}

DottedTextLayer *layer_factory_create_dotted_text_layer(
	LayerBuilder builder,
	GColor text_color,
	bool align_right,
	const char *initial_text
) {
	DottedTextLayer *layer = dotted_text_layer_create(builder.bounds);

	dotted_text_layer_set_color(layer, text_color);
	dotted_text_layer_set_align_right(layer, align_right);
	if (initial_text) {
		dotted_text_layer_set_text(layer, (char *) initial_text);
	}

	layer_add_child(builder.parent, layer);

	return layer;
}

Layer *layer_factory_create_custom_layer(
	LayerBuilder builder,
	LayerUpdateProc update_proc
) {
	Layer *layer = layer_create(builder.bounds);
	layer_set_update_proc(layer, update_proc);
	layer_add_child(builder.parent, layer);

	return layer;
}

