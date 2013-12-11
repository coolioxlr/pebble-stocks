/*
 * File: app/view/MyPanel.js
 *
 * This file was generated by Sencha Architect version 3.0.0.
 * http://www.sencha.com/products/architect/
 *
 * This file requires use of the Sencha Touch 2.3.x library, under independent license.
 * License of Sencha Architect does not include license for Sencha Touch 2.3.x. For more
 * details see http://www.sencha.com/license or contact license@sencha.com.
 *
 * This file will be auto-generated each and everytime you save your project.
 *
 * Do NOT hand edit this file.
 */

Ext.define('AddStockApp.view.MyPanel', {
    extend: 'Ext.Panel',

    config: {
        scrollable: 'vertical',
        items: [
            {
                xtype: 'titlebar',
                docked: 'top',
                title: 'Define your stocks',
                items: [
                    {
                        xtype: 'button',
                        handler: function(button, e) {
                            var stocks = [];

                            var inputFieldContainer = Ext.getCmp('inputFieldContainer');





                            for(i=0; i<inputFieldContainer.items.length;i++){



                                var container = inputFieldContainer.getAt(i);

                                var textField = container.getAt(0);

                                var value = textField.getValue( );
                                stocks[i] = value;
                            }


                            var pebbleSting = stocks.join(",");
                            console.log (pebbleSting);


                            window.location.href = 'pebblejs://close#' + pebbleSting;

                        },
                        align: 'right',
                        text: 'Done'
                    }
                ]
            },
            {
                xtype: 'fieldset',
                id: '',
                title: 'My Stocks',
                items: [
                    {
                        xtype: 'container',
                        id: 'inputFieldContainer',
                        layout: {
                            type: 'vbox'
                        },
                        items: [
                            {
                                xtype: 'container',
                                centered: false,
                                margin: '',
                                layout: {
                                    type: 'hbox'
                                },
                                items: [
                                    {
                                        xtype: 'textfield',
                                        margin: '',
                                        maxWidth: '',
                                        width: 258,
                                        label: 'Stock #1',
                                        labelWidth: '50%',
                                        autoCapitalize: true,
                                        autoComplete: true,
                                        autoCorrect: true,
                                        maxLength: 4,
                                        placeHolder: ''
                                    },
                                    {
                                        xtype: 'container',
                                        width: 61,
                                        layout: {
                                            type: 'vbox'
                                        },
                                        items: [
                                            {
                                                xtype: 'button',
                                                handler: function(button, e) {
                                                    var container = button.getParent().getParent();
                                                    container.destroy();



                                                },
                                                centered: true,
                                                docked: 'right',
                                                margin: 0,
                                                maxWidth: '',
                                                width: '',
                                                icon: 'true',
                                                iconCls: 'delete'
                                            }
                                        ]
                                    }
                                ]
                            }
                        ]
                    }
                ]
            },
            {
                xtype: 'container',
                height: 40
            },
            {
                xtype: 'button',
                handler: function(button, e) {
                    var fieldSet = Ext.getCmp('inputFieldContainer');

                    var name = "Stock #"+(fieldSet.items.length+1);
                    var newContainer = Ext.create('Ext.Container', {
                        xtype: 'container',
                        centered: false,
                        margin: '',
                        layout: {
                            type: 'hbox'
                        },
                        items: [
                        {
                            xtype: 'textfield',
                            margin: '',
                            maxWidth: '',
                            width: 258,
                            label: name,
                            labelWidth: '50%',
                            autoCapitalize: true,
                            autoComplete: true,
                            autoCorrect: true,
                            maxLength: 4,
                            placeHolder: ''
                        },
                        {
                            xtype: 'container',
                            width: 61,
                            layout: {
                                type: 'vbox'
                            },
                            items: [
                            {
                                xtype: 'button',
                                handler: function(button, e) {
                                    var container = button.getParent().getParent();
                                    container.destroy();

                                    var inputFieldContainer = Ext.getCmp('inputFieldContainer');





                                    for(i=0; i<inputFieldContainer.items.length;i++){



                                        var container = inputFieldContainer.getAt(i);

                                        var textField = container.getAt(0);
                                        textField.setLabel( "Stock #"+(i+1));
                                    }
                                },
                                centered: true,
                                docked: 'right',
                                margin: 0,
                                maxWidth: '',
                                width: '',
                                icon: 'true',
                                iconCls: 'delete'
                            }
                            ]
                        }
                        ]
                    });


                    fieldSet.add(newContainer);


                },
                text: 'Add more'
            },
            {
                xtype: 'container',
                height: 100
            }
        ]
    }

});