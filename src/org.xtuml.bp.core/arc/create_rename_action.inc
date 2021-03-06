.//=====================================================================
.// 
.// (c) Copyright 2005-2014 by Mentor Graphics Corp. All rights reserved.
.//
.//=====================================================================
.// Licensed under the Apache License, Version 2.0 (the "License"); you may not
.// use this file except in compliance with the License.  You may obtain a copy
.// of the License at
.//
.//      http://www.apache.org/licenses/LICENSE-2.0
.//
.// Unless required by applicable law or agreed to in writing, software
.// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
.// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   See the
.// License for the specific language governing permissions and limitations under
.// the License.
.//=====================================================================
.//
.// The function declared below is responsible for creating the core.ui
.// action for rename. It is declared here because multiple plugins use
.// the same actions to carry out this task on the metamodel. Takes a
.// path parameter so that the comment correctly displays the generated
.// files location.
.//
.include "./arc/parse_chain.inc"
.function create_rename_action
  .param string path
  .//
  .assign action_name = "RenameAction"
  .assign attr_result = action_name
package org.xtuml.bp.core.ui;
//======================================================================
//
// File: ${path}/${action_name}.java
//
// WARNING:      Do not edit this generated file
// Generated by: ${info.arch_file_name}
// Version:      $$Revision: 1.45 $$
//
// (c) Copyright 2005-2014 by Mentor Graphics Corp.  All rights reserved.
//
//======================================================================
//
// This class is the main BridgePoint entry point for the retargetable
// action, Rename.
//
import java.util.Iterator;
import java.util.Collection;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.dialogs.InputDialog;
import org.eclipse.jface.dialogs.MessageDialogWithToggle;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.TreeEditor;
import org.eclipse.swt.events.FocusAdapter;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.swt.widgets.TreeItem;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.actions.TextActionHandler;
import org.xtuml.bp.core.util.UIUtil;

import org.xtuml.bp.core.common.*;
import org.xtuml.bp.core.*;
import org.xtuml.bp.core.inspector.ModelInspector;
import org.xtuml.bp.core.util.TransactionUtil;

public class ${action_name} extends Action {
  private TreeEditor treeEditor;
  private TreeViewer treeViewer;
  private Text textEditor;
  private Composite textEditorParent;
  private TextActionHandler textActionHandler;
  private Object inlinedSelection;
  public RenameAction(TreeViewer treeViewer){
    super(ActionFactory.RENAME.getId());
    this.treeViewer = treeViewer;
    this.treeEditor = new TreeEditor(treeViewer.getTree());
    setText("Rename");
    setToolTipText("Rename this model Element");
  }
  public void run() {
    IStructuredSelection structuredSelection = Selection.getInstance().getStructuredSelection();
    Iterator iterator = structuredSelection.iterator();
    if (iterator.hasNext()) {
      Object selection = iterator.next();
      queryNewResourceNameInline(selection);
    }
  }
  Composite createParent() {
    Tree tree = getTree();
    Composite result = new Composite (tree, SWT.NONE);
    TreeItem[] selectedItems = tree.getSelection();
    treeEditor.horizontalAlignment = SWT.LEFT;
    treeEditor.grabHorizontal = true;
    treeEditor.setEditor(result, selectedItems[0]);
    return result;
  }
  /**
   * Create the text editor widget.
   *
   * @param resource the resource to rename
   */
  private void createTextEditor(final Object resource) {
    // Create text editor parent.  This draws a nice bounding rect.
    textEditorParent = createParent();
    textEditorParent.setVisible(false);
    textEditorParent.addListener(SWT.Paint, new Listener() {
      public void handleEvent (Event e) {
        Point textSize = textEditor.getSize();
        Point parentSize = textEditorParent.getSize();
        e.gc.drawRectangle(0, 0, Math.min(textSize.x + 4, parentSize.x - 1), parentSize.y - 1);
      }
    });

    // Create inner text editor.
    textEditor = new Text(textEditorParent, SWT.NONE);
    textEditorParent.setBackground(textEditor.getBackground());
    textEditor.addListener(SWT.Modify, new Listener() {
      public void handleEvent (Event e) {
        Point textSize = textEditor.computeSize(SWT.DEFAULT, SWT.DEFAULT);
        textSize.x += textSize.y; // Add extra space for new characters.
        Point parentSize = textEditorParent.getSize();
        textEditor.setBounds(2, 1, Math.min(textSize.x, parentSize.x - 4), parentSize.y - 2);
        textEditorParent.redraw();
      }
    });
    textEditor.addListener(SWT.Traverse, new Listener() {
      public void handleEvent(Event event) {

        //Workaround for Bug 20214 due to extra
        //traverse events
        switch (event.detail) {
          case SWT.TRAVERSE_ESCAPE:
            //Do nothing in this case
            disposeTextWidget();
            event.doit = true;
            event.detail = SWT.TRAVERSE_NONE;
            break;
          case SWT.TRAVERSE_RETURN:
            saveChangesAndDispose(resource);
            event.doit = true;
            event.detail = SWT.TRAVERSE_NONE;
            break;
        }
      }
    });
    textEditor.addFocusListener(new FocusAdapter() {
      public void focusLost(FocusEvent fe) {
        saveChangesAndDispose(resource);
      }
    });

    if (textActionHandler != null)
      textActionHandler.addText(textEditor);
  }
  /**
   * Close the text widget and reset the editorText field.
   */
  private void disposeTextWidget() {
    if (textActionHandler != null)
      textActionHandler.removeText(textEditor);

    if (textEditorParent != null) {
      textEditorParent.dispose();
      textEditorParent = null;
      textEditor = null;
      treeEditor.setEditor(null,null);
    }
  }
  /**
   * Get the Tree being edited.
   * @returnTree
   */
  public Tree getTree() {
    return this.treeViewer.getTree();
  }
  /**
   * Get the TextEditor being edited. Only used by unit test
   * @return Text
   */
  public Text getTextEditor()
  {
    return textEditor;
  }
  /**
   * Return the new name to be given to the target resource or <code>null<code>
   * if the query was canceled. Rename the currently selected resource using the table editor.
   * Continue the action when the user is done.
   *
   * @return java.lang.String
   * @param resource the resource to rename
   */
  private void queryNewResourceNameInline(final Object selection) {
    // Make sure text editor is created only once. Simply reset text
    // editor when action is executed more than once. Fixes bug 22269.
    if (textEditorParent == null) {
      createTextEditor(selection);
    }
    textEditor.setText(getEditorText(selection));

    // Open text editor with initial size.
    textEditorParent.setVisible(true);
    Point textSize = textEditor.computeSize(SWT.DEFAULT, SWT.DEFAULT);
    textSize.x += textSize.y; // Add extra space for new characters.
    Point parentSize = textEditorParent.getSize();
    textEditor.setBounds(2, 1, Math.min(textSize.x, parentSize.x - 4), parentSize.y - 2);
    textEditorParent.redraw();
    textEditor.selectAll ();
    textEditor.setFocus ();
  }
  /**
   * Save the changes and dispose of the text widget.
   * @param resource - the resource to move.
   */
  private void saveChangesAndDispose(final Object selection) {
    // Cache the resource to avoid selection loss since a selection of
    // another item can trigger this method
    inlinedSelection = selection;
    final String newName = textEditor.getText();
    // Run this in an async to make sure that the operation that triggered
    // this action is completed.  Otherwise this leads to problems when the
    // icon of the item being renamed is clicked (i.e., which causes the rename
    // text widget to lose focus and trigger this method).
    Runnable query = getRenameQuery(
      selection,
      newName,
      getEditorText(selection), true);
        //Dispose the text widget regardless
        disposeTextWidget();
    getTree().getShell().getDisplay().asyncExec(query);
    inlinedSelection = null;
  }
  public static Runnable getRenameQuery(
    final Object selection,
    final String newName,
    final String oldName,
    final boolean displayWarning) {
    return new Runnable() {
      public void run() {
        boolean success = false;
        if (!newName.equals(oldName)) {
          String message = isNameValid(newName, (ModelElement)selection);
          if (message != null && displayWarning) {
            UIUtil.openWarning(null, "Invalid Name", message);
            return;
          }
          if (!TransactionUtil.modifySelectedResources(
                            "Confirm Element Rename",
                            "The selected element is read only.  Do you still wish to rename it?"))
                  return;
          IStructuredSelection structuredSelection = new StructuredSelection(selection);
  .assign test = "if"
  .select many menu_entries from instances of CME
  .for each menu_entry in menu_entries
    .if ("$lr{menu_entry.Specialism}" == "rename")
      .select one class related by menu_entry->O_OBJ[R2000]
      .invoke cn = get_class_name(class)
          ${test} (selection instanceof ${cn.body}) {
      .invoke fn = get_func_name(menu_entry)
      .// the following line actually invokes the nominated function generated from the client model
            success = ${fn.body}(structuredSelection, newName);
          }
      .assign test = "else if"
    .end if
  .end for
                        }
                    }
    };
  }
  .for each menu_entry in menu_entries
    .if ("$lr{menu_entry.Specialism}" == "rename")
      .select one class related by menu_entry->O_OBJ[R2000]
      .invoke cn = get_class_name(class)
      .invoke fn = get_func_name(menu_entry)
  private static boolean ${fn.body}(IStructuredSelection  selection, String p_New_name) {
      .select any function from instances of S_SYNC where ("${selected.Name}" == "${fn.body}")
      .if (not_empty function)
        .if ( class.AdapterName == "IProject" )
    ${cn.body} element = (${cn.body}) selection.iterator().next();
    IStatus name_ok = validateSystemModelName(element, p_New_name);
    if (!name_ok.isOK()) {
        notifyUserOfNameConflict(name_ok);
        return false;
    }
        .elif ( class.AdapterName == "IFile" )
          .select any child_node from instances of T_TNS where (selected.Key_Lett == class.Key_Lett)
    ${cn.body} element = (${cn.body}) selection.iterator().next();
    IStatus name_ok = validateComponentName(element, p_New_name);
    if (!name_ok.isOK()) {
        notifyUserOfNameConflict(name_ok);
        return false;
    }
        .end if
        .invoke result = function_body(function)
${result.body}
      .else
        .print "ERROR: No function, ${fn.body}, found for context menu entry: Specialism: ${menu_entry.Specialism}  Label: ${menu_entry.Label}  Key_Lett: ${menu_entry.Key_Lett}"
      .end if
    return true;
  }
    .end if
  .end for

    public static IStatus validateSystemModelName(SystemModel_c systemModel, String new_name) {
        return validateName(systemModel, new_name, IResource.PROJECT);
    }

    public static IStatus validateComponentName(NonRootModelElement element, String new_name) {
        return validateName(element, new_name, IResource.FILE | IResource.FOLDER);
    }

    public static IStatus validateName(NonRootModelElement element, String new_name, int type) {
        IPersistenceHierarchyMetaData hmd = PersistenceManager.getHierarchyMetaData();
        if (hmd.isComponentRoot(element)) {
            IStatus nameOK = CorePlugin.getWorkspace().validateName(new_name, type);

            if (nameOK.isOK()) {
                /* new_name must not begin or end with a whitespace */
                if (Character.isWhitespace(new_name.charAt(0))
                    || Character.isWhitespace(new_name.charAt(new_name.length() - 1))) {
                    String name = CorePlugin.getDefault().getBundle().getSymbolicName();
                    return new Status(IStatus.ERROR, name, 1, "Resource name cannot begin or end with whitespace", null);
                }

                // still need to check for duplicate name
                if (type == IResource.PROJECT) {
        IProject[] sys_set = CorePlugin.getWorkspace().getRoot().getProjects();
                    IProject selection = (IProject) ((SystemModel_c) element).getAdapter(IProject.class);
        for (int i = 0; i < sys_set.length; ++i) {
            if (selection != sys_set[i]) {
                if (CorePlugin.osIsCaseInsensitive()) {
                                if (new_name.toLowerCase().equals((sys_set[i]).getName().toLowerCase())) {
                                    String name = CorePlugin.getDefault().getBundle().getSymbolicName();
                                    return new Status(IStatus.ERROR, name, 1, "Resource already exists with that name", null);
                    }
                } else {
                                if (new_name.equals((sys_set[i]).getName())) {
                                    String name = CorePlugin.getDefault().getBundle().getSymbolicName();
                                    return new Status(IStatus.ERROR, name, 1, "Resource already exists with that name", null);
                    }
                }
            }
        }
                } else {
					String name = CorePlugin.getDefault().getBundle()
							..getSymbolicName();
					if (PersistenceManager.getDefaultInstance().nameIsUnique(
							element, new_name)) {
						// in some cases the name may not be unique, this
						// happens when dealing with a team provider like
						// SVN.  Old folders may exist that are scheduled
						// for deletion, so we need to warn if the resulting
						// folder exists
						IFile file = element.getFile();
						if (file == null) {
							// this happens during name validation at creation time
							// we need to build the file path and create an in-memory
							// workspace file
							NonRootModelElement parent = hmd.getParent(element);
							while (parent.getFile() == null) {
								parent = hmd.getParent(parent);
							}
							if (parent != null) {
								IPath fullPath = parent.getFile().getFullPath()
										..removeLastSegments(1);
								fullPath = fullPath.append(new_name)
										..addTrailingSeparator().append(
												new_name + "."
														+ Ooaofooa.MODELS_EXT);
								file = ResourcesPlugin.getWorkspace().getRoot()
										..getFile(fullPath);
							}
						}
						IPath newPath = file.getFullPath()
								..removeLastSegments(2).append(new_name);
						IFolder folder = ResourcesPlugin.getWorkspace()
								..getRoot().getFolder(newPath);
						if (folder.exists() && !file.exists()) {
							return new Status(
									IStatus.ERROR,
									name,
									1,
									"A model element with the same name already exists.\n\n"
											+ "This is most likely due to uncommitted renames or deletions."
											+ "  If this is the case, please commit any outstanding changes before proceeding.\n\n"
											+ "If the project is not connected to a configuration management system"
											+ ", you will need to remove the target manually.",
									null);
						}
						return Status.OK_STATUS;
					} else {
						return new Status(IStatus.ERROR, name, 1,
								"Resource already exists with that name", null);
					}
                }
                } else {
                // there was a problem
                return nameOK;
                    }
                }
        return Status.OK_STATUS;
            }

    private static void notifyUserOfNameConflict(IStatus stat) {
        org.eclipse.swt.widgets.Shell sh = CorePlugin.getDefault().getWorkbench().getActiveWorkbenchWindow().getShell();
        org.eclipse.jface.dialogs.MessageDialog.openError(sh, "Name Conflict",
                stat.getMessage());
        }

    /**
     * This method returns a string which does not
     * contain any label related values.  This is
     * necessary when an element is represented by
     * a label which contains more than the elements
     * name.
     *
     * @param Object the currently selected element
     * @return String the string used in the text editor
     */
    private String getEditorText(Object element) {

    .// generate the calls which are made to the model
    .// element's naming attributes, so that the correct
    .// text is used in the in-place editor
    .select many tree_nodes from instances of T_TNS
    .for each node in tree_nodes
      .select any node_class from instances of O_OBJ where (selected.Key_Lett == node.Key_Lett)
      .invoke ncn = get_class_name(node_class)
      .if (first tree_nodes)
    if (element instanceof ${ncn.body}) {
      .else
    else if (element instanceof ${ncn.body}) {
      .end if
      .if ("${node.Label}" != "")
      return "${node.Label}";
      .elif ("${node.NamingAttributeRelChain}" != "")
        .// We need to navigate to a different instance to obtain the name to use
        .// First obtain the class at the end of the navigation
        .invoke nav_string = generate_backward_rel_chain_nav( node, node.NamingAttributeRelChain, "one", "element", "result" )
${nav_string.body}\
        .// emit the result variable ready to have the accessor tacked on
        .invoke aa = get_attribute_accessor_from_str( node.NamingAttribute )
      String text = result.${aa.body}();
      if (text == null) {
        return "";
      }
      else {
        return text;
      }
      .else
        .// omit non-label text in the case of
        .// a label which contains more than
        .// just the instances name
        .if(node.NonLabelNamingAttribute != "")
          .invoke aa = get_attribute_accessor_from_str( node.NonLabelNamingAttribute )
      String text = ((${ncn.body})element).${aa.body}();
      if(text == null)
        return "";
      else
        return text;
        .elif(node.NamingAttribute != "")
          .invoke aa = get_attribute_accessor_from_str( node.NamingAttribute )
        String text = ((${ncn.body})element).${aa.body}();
        if (text == null) {
          return "";
        }
        else {
          return text;
        }
        .end if
      .end if
    }
    .end for
    return "Unknown Model Element";
    }

    static public boolean can${action_name}()
    {
        PersistenceManager pm = PersistenceManager.getDefaultInstance();
        PersistableModelComponent component = null;

        boolean result = false;
        final IStructuredSelection structuredSelection =
            Selection.getInstance().getStructuredSelection();
        if (structuredSelection.size() > 1) {
            return false;
        }
        Object context = structuredSelection.getFirstElement();
        component = pm.getComponent((NonRootModelElement)context);
  .assign else_str = ""
  .for each menu_entry in menu_entries
    .if ("$lr{menu_entry.Specialism}" == "rename")
      .select one class related by menu_entry->O_OBJ[R2000]
      .invoke cn = get_class_name(class)
        ${else_str}if (context instanceof ${cn.body})
        {
                  .select any op related by class->O_TFR[R115] where (selected.Name == "canRename")
                  .if (not_empty op)
                    result = ((${cn.body})context).Canrename();
                  .else
                    result = true;
                  .end if
                }
      .assign else_str = "else "
    .end if
  .end for
        else
        {
            result = false;
        }
        return result;
    }

    /**
     * Returns null if the given name is valid for the given model element.
     * Otherwise, returns a message stating why the name is invalid.
     */
    public static String isNameValid(String name, ModelElement element)
    {

    	if (CorePlugin.getDefault().getPreferenceStore().getBoolean(BridgePointPreferencesStore.REQUIRE_MASL_STYLE_IDENTIFIERS))
    	{
			if ( element instanceof UserDataType_c ) {
				// Due to the current limitations of the xtUML type system, for MASL the
				// modeler creates UDTs to represent complex MASL types.  These are too hard
				// for this simple character validator to deal with, so just let all UDT names through.
				return null;
			}
			if ( element instanceof StateMachineEvent_c ) {
				// Events may be of the form <super class name>.<super event name> so we allow a single dot by
				// just removing the first dot we see from the name being validated.
				name = name.replaceFirst("\\.", "");
			}
    		// Make sure the name conforms to MASL identifier rule:
    		//   ( Letter | '_' )( Letter | Digit | '_' )*
			if ( name.isEmpty() ) {
				return CorePlugin.INVALID_MASL_NAME;
			}
    		int i = 0;
    		if ( !(Character.isLetter(name.charAt(i)) || name.charAt(i) == '_') ) {
    		  return CorePlugin.INVALID_MASL_NAME;
    		}
    		for (i = 1; i < name.length(); i++) {
        		if ( !(Character.isLetterOrDigit(name.charAt(i)) || name.charAt(i) == '_') ) {
        		  return CorePlugin.INVALID_MASL_NAME;
        		}
    		}
    	} else {
        // if the given element is of a type whose instances may not have
        // spaces in their names, and there is a space in the given name
        if ((element instanceof Attribute_c
            || element instanceof FunctionParameter_c
            || element instanceof OperationParameter_c
            || element instanceof BridgeParameter_c
            || element instanceof Function_c
            || element instanceof Operation_c
            || element instanceof Bridge_c
            || element instanceof StateMachineEventDataItem_c
            || element instanceof DataType_c
            || element instanceof Enumerator_c
            || element instanceof SymbolicConstant_c
            || element instanceof LiteralSymbolicConstant_c
			|| element instanceof StructureMember_c
			|| element instanceof InterfaceOperation_c
			|| element instanceof InterfaceSignal_c
			|| element instanceof SignalEvent_c
			|| element instanceof MessageArgument_c)
            && name.contains(" ")) {
            // the name is not valid
            return CorePlugin.INVALID_NAME_SPACES;
        }
    	}

    if (null != element) {
        final ModelRoot modelRoot = ((NonRootModelElement) element).getModelRoot();

                    .select many sig_op_set from instances of O_TFR where (selected.name == "getSignature")
                    .select many sig_obj_set related by sig_op_set->O_OBJ[R115]
                    // TODO - LPS
                    // This code ignores model elements that can have a signature (have the 'getSignature' operation).
                    // In the future we should check that the signatures are unique instead of the names. For now we are
                    // simply ignoring the name check for these elements.
        if ( 
                    .for each sig_obj in sig_obj_set
                      .invoke cn = get_class_name(sig_obj)
                       !(element instanceof ${cn.body}) &&
                    .end for
                        !(Ooaofooa.Getuniqueinitialname(modelRoot, name, element).equals(name))) {
          return CorePlugin.DUPLICATE_NAME_ERROR;
        }
    }
		
        return null;
    }
	
	public static void handleRename(Object selection, String oldName, Shell sh) {
			InputDialog id = new InputDialog(sh, "Rename",
					"Enter the new name:", oldName, UIUtil.newRenameValidator((ModelElement) selection));
			int result = id.open();
			if (result == InputDialog.OK) {
				String newName = id.getValue();
				// Run this in an async to make sure that the operation that
				// triggered this action is completed. Otherwise this
				// leads to problems when the icon of the item being renamed
				// is clicked (i.e., which causes the rename text widget
				// to lose focus and trigger this method).
				Runnable query = RenameAction.getRenameQuery(selection,
						newName, oldName, false);
				if (sh != null) {
					sh.getDisplay().asyncExec(query);
				} else {
					PlatformUI.getWorkbench().getDisplay().asyncExec(query);
				}
			}
	}
} // end ${action_name}
.end function
.//
.//
.//
